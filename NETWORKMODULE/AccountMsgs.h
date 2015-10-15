#pragma once
#include "NetModule.h"
#include "../pub/ConstValue.h"

// �˺����ݿ��������
// ��½ʱ�����˺� [EPRO_CHECK_ACCOUNT] �����ڽ����˺ſ�ʼ�Ƿ�ʱ����Ϊ�ͻ���������Ч�����Լ��ɹ���ͬʱ�˺ſ�ʼ�Ƿѣ�ʵ�����Ǽ�¼��½ʱ�䣬�ȵ��˳���ʱ������ݿ�۵㣩
// �Ƿѹ����У���ʱ���Է���Ϣ [EPRO_REFRESH_CARD_POINT] ���˺ŷ�������������ˢ�£�Ҳ���Ǹ�����Ϸʱ����п۵㣬ͬʱ��Ϸʱ�����㣬������˺Ż�δ�����Ƿѣ����Զ���ʼ�Ƿ�
// �Ƿѹ����У��˺ŷ�������ʱ����ѯ����Ϸ������ [ ] ĳ���˺��Ƿ���Ȼ���ڣ����ѯ��ʧ�ܣ���Ϸ���������ϻ���˺Ų���������Ϸ�����������Ƿ�ֹͣ
// ����˳�ʱ����Ϸ������֪ͨ�˺ŷ����� [EPRO_ACCOUNT_LOGOUT] �˺��˳��� �Ƿ�ֹͣ

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// �˺ŷ����������Ϣ��
//=============================================================================================
DECLARE_MSG_MAP(SAccountBaseMsg, SMessage, SMessage::EPRO_ACCOUNT_MESSAGE)
//{{AFX
EPRO_REGISTER_LOGINSERVER,
EPRO_CHECK_PASSWORD,        // ����ʺ�����
EPRO_CHECK_ACCOUNT,         // ����˺Ų�������
EPRO_ACCOUNT_LOGOUT,        // �˺��˳�
EPRO_REFRESH_CARD_POINT,    // ����ˢ��
EPRO_PLAYER_ADD_ACCOUNT,    // ...
EPRO_FORCE_LOGOUT,
//GM����
EPRO_CHANGE_PASSWORD,       // ���������
ERPO_BLOCK_ACCOUNT,
EPRO_GETONLINETIME,
EPRO_GMCHECK_ACCOUNT,
EPRO_GMLOGOUT,
EPRO_EXCHANGECARDPOINT,     // ��ҽ��׵���
EPRO_ACCOUNT_HANGUP,        // ��ҿ�ʼ�һ�
EPRO_REFRESH_MONEY_PIONT,	// Ԫ������ˢ��
EPRO_GET_ACCOUNT_MDC_ID,    // ��ȡ����ڱ�������ͨ��Ѷ���޹�˾�������ID��
EPRO_QUERY_ENCOURAGEMENT, //�齱��ѯ
EPRO_CHECK_PLAYEREXIST,
EPRO_UPDATE_SERVER_STATE,   // ���·�����״̬
EPRO_UPDATE_TIMELIMIT,      // ʱ���������,���ڷ�����ϵͳ!
//}}AFX
END_MSG_MAP_WITH_ROTOC()

DECLARE_MSG( _SUpdateServerState, SAccountBaseMsg, SAccountBaseMsg::EPRO_UPDATE_SERVER_STATE )
struct SUpdateServerState :
    public _SUpdateServerState
{
    DWORD ServerID;
    DWORD PlayerNumber;
    DWORD ServerState;
	//10-06-08
	WORD  TotalMac;       // MAC������
	//��չ����
	char streamData[ MAX_ONLINEEXTEND ];
};

DECLARE_MSG(SGMAccountLogoutBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_GMLOGOUT)
struct SQGMAccountLogoutMsg :
    public SGMAccountLogoutBaseMsg
{
    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};

struct SAGMAccountLogoutMsg :
    public SGMAccountLogoutBaseMsg
{
    enum
    {
        RET_SUCCESS,
        RET_INVALID_ACCOUNT,        // ��Ч���˺�
        RET_INVALID_PASSWORD,       // ��Ч������
        RET_BADINFO,
        LOGOUT_ERROR_NOTLOGIN,      // Player Logout without login ,some thing wrong happend 
        LOGOUT_ERROR_NORIGHT,
        LOGOUT_ERROR_CATCHERROR,    // 
        LOGOUT_ERROR_NOSESSIONID,
        LOGOUT_ERROR_UNHANDLED
    };

    BYTE byResult;
};



//=============================================================================================
// �����˺ż�����Ϣ��ͨ���˺���&�˺��������ж��Ƿ�Ϊ�Ϸ��û���ͬʱ����������ݣ�ʣ�������
DECLARE_MSG(SGMCheckAccountBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_GMCHECK_ACCOUNT)
struct SQGMCheckAccountMsg :
    public SGMCheckAccountBaseMsg
{
    int  iServerID;
    char szIP[17];

    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};

struct SAGMCheckAccountMsg :
    public SGMCheckAccountBaseMsg
{
    enum
    {
        RET_SUCCESS,                // �ɹ�
        RET_INVALID_ACCOUNT,        // ��Ч���˺�
        RET_INVALID_PASSWORD,       // ��Ч������
        RET_NOCARDPOINT,            // �㿨ֵ����
        RET_LOGGEDIN,               // �Ѿ����ڵ�¼״̬
        RET_BADINFO,                // �쳣��Ϣ
        RET_BLOCKED,
    };

    BYTE byResult;
    DWORD dwCardPoint;              // ʣ��ĵ���
	//DWORD dwMoneyPoint;				// Ԫ������

	// ��������ADDHERE
	//DWORD dwXYDPoint;
    QWORD qwPuid;                   // �ʺŵ�ΨһID��֮�����滻 dwMoneyPoint �� dwXYDPoint ������ dwGiftCoupon �� dwXYDPoint ��Ϊ�˽�����ݶ�������⣡
	DWORD dwGiftCoupon;

    WORD  wGMLevel;

    // �˺ţ����룬2������
	char streamData[ MAX_ACCAPASS + CONST_USERPASS ];
};
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseBAMsg,SAccountBaseMsg,SAccountBaseMsg::ERPO_BLOCK_ACCOUNT)
struct SQBAMsg : 
    public SBaseBAMsg
{
    DWORD dwBlockTime;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SABAMsg :
    public SBaseBAMsg
{
    DWORD dwBlockTime;
    WORD  wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGOTMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_GETONLINETIME)
struct SQGOTMsg :
    public SBaseGOTMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
struct SAGOTMsg :
    public SBaseGOTMsg
{
    int iOnlineTime;
    WORD  wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SChangePasswordMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_CHANGE_PASSWORD)
struct SQChangePasswordMsg :
    public SChangePasswordMsg
{
    // �˺ţ����룬������
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};

struct SAChangePasswordMsg :
    public SChangePasswordMsg
{
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_WRONGPASS,
        RT_ERROR    
    };
};

DECLARE_MSG(SCheckPasswordMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_CHECK_PASSWORD)
struct SQCheckPasswordMsg :
    public SCheckPasswordMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPasswordMsg :
    public SCheckPasswordMsg
{
    WORD wResult;
};


DECLARE_MSG(SForceLogoutMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_FORCE_LOGOUT)
struct SQForceLogoutMsg :
    public SForceLogoutMsg
{
    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};

struct SAForceLogoutMsg :
    public SForceLogoutMsg
{
    WORD wResult;
};


//=============================================================================================
// ���˺ŷ�������ע���¼������
DECLARE_MSG(SRegisterLogsrvBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_REGISTER_LOGINSERVER)
struct SQRegisterLogsrvMsg :
    public SRegisterLogsrvBaseMsg
{
    DWORD IP;
    DWORD ID;
};

struct SARegisterLogsrvMsg :
    public SRegisterLogsrvBaseMsg
{
    enum
    {
        RET_SUCCESS,
        RET_BADINFO,
    };

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------

// �����˺ż�����Ϣ��ͨ���˺���&�˺��������ж��Ƿ�Ϊ�Ϸ��û���ͬʱ����������ݣ�ʣ�������
DECLARE_MSG(SCheckAccountBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_CHECK_ACCOUNT)
struct SQCheckAccountMsg : public SCheckAccountBaseMsg
{
    int  iServerID; 
    char szIP[17];

    // �̶�����     �� �˺ţ����룬macAddress
    // ��չ����д�� �� �ڹ̶������󸽼Ӽ�ֵ�ԣ�������Ϊ�ַ����������Ĵ洢���̲������� @PlayerAccount�� ֵΪ��Ӧ�������ͣ�
	char streamData[MAX_ACCAPASS];
};

struct SACheckAccountMsg : public SCheckAccountBaseMsg
{
    enum
    {
        RET_SUCCESS,                // �ɹ�
        RET_INVALID_ACCOUNT,        // ��Ч���˺�
        RET_INVALID_PASSWORD,       // ��Ч������
        RET_NOCARDPOINT,            // �㿨ֵ����
        RET_LOGGEDIN,               // �Ѿ����ڵ�¼״̬
        RET_BADINFO,                // �쳣��Ϣ
        RET_BLOCKED,                // �����ʺ�
        RET_LIMITED,                // �����Ƶ��˺�
        RET_SUCCESS_WITH_ITEM,      // �ɹ������и��ӵ��ߴ���
		RET_LOGIN_OTHER_SERVER      // ����������������¼
    };

	SACheckAccountMsg()
	{
		dwCardPoint = 0;              // ʣ��ĵ���
        qwPuid = 0;
		dwGiftCoupon = 0;
	}

    BYTE	byResult;
    DWORD	dwCardPoint;              // ʣ��ĵ���
    QWORD	qwPuid;                   // �ʺŵ�ΨһID
	DWORD	dwGiftCoupon;
    WORD	wGMLevel;

    // �˺ţ����룬2�����룬������������
	char streamData[ MAX_ACCAPASS + CONST_USERPASS + MAX_USERDESC ];
};


//=============================================================================================
// ���ڼ���ɫ�����Ƿ���ڵ���Ϣ
DECLARE_MSG(SCheckPlayerExistBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_CHECK_PLAYEREXIST)
struct SQCheckPlayerExistBaseMsg :
    public SCheckPlayerExistBaseMsg
{
	SQCheckPlayerExistBaseMsg()
	{
		iDestServerID = 0;
	}

	int  iDestServerID;

    // �˺ţ�����
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPlayerExistBaseMsg :
    public SCheckPlayerExistBaseMsg
{
	SACheckPlayerExistBaseMsg()
	{
		iSrcServerID = 0;
		iDestServerID = 0;
		wExist = 1;
	}

    int  iSrcServerID;
	int  iDestServerID;
	WORD wExist;	// 0δ�ҵ��� 1�����Ѵ���

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

//=============================================================================================
// ����ˢ��Ŀ���˻���ʣ�����
DECLARE_MSG(SRefreshCardBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_REFRESH_CARD_POINT)
struct SQRefreshCardMsg :
    public SRefreshCardBaseMsg
{
    enum
    {
        QUEST_REFRESH,      // �������ˢ�£�
        QUEST_DEL,          // ����۵㣡
        QUEST_EXCHANGE,     // ����׼�����ף�
        QUEST_ACCHANGE,     // ����ȷ�Ͻ��ף�
        QUEST_CHKITEM,      // ����׼�����ӵ��ߣ�
        QUEST_ACCITEM,      // ����ȷ�ϸ��ӵ��ߣ�
        // ���׹�����������ͨ��������ȷ�ϵ��������ز����룡���˺ŷ�����log�����׼�¼�Ͳ����룡��
        // Ȼ��ͨ��ȷ�Ͻ��ף��жϲ����룬����������ָ���˺ţ������������logȷ�Ͻ��׼�¼�Ͳ����룡��
    };

    BYTE byRefreshType;     // ˢ�·�ʽ
    int iCardPoint;         // �ı�ĵ���
    DWORD dwActionCode;     // �����루�ش���

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SARefreshCardMsg :
    public SRefreshCardBaseMsg
{
    enum
    {
        QUEST_REFRESH,      // �������ˢ�£�
        QUEST_DEL,          // ����۵㣡
        QUEST_EXCHANGE,     // �����ף�
        QUEST_ACCHANGE,     // ȷ�Ͻ��ף�
        QUEST_CHKITEM,      // ����׼�����ӵ��ߣ�
        QUEST_ACCITEM,      // ����ȷ�ϸ��ӵ��ߣ�
        // ���׹�����������ͨ��������ȷ�ϵ��������ز����룡���˺ŷ�����log�����׼�¼�Ͳ����룡��
        // Ȼ��ͨ��ȷ�Ͻ��ף��жϲ����룬����������ָ���˺ţ������������logȷ�Ͻ��׼�¼�Ͳ����룡��
    };

    enum 
    {
        RET_SUCCESS,            // �����ɹ�
        RET_INVALID_ACCOUNT,    // ��Ч���˺�
        RET_POINT_NOTENOUGH,    // û���㹻�ĵ㿨
        RET_BADINFO,            // ����ʧ�ܣ�������Ϣ�ķ���
        RET_ITEM_NOTEXIST,      // �����ڸ��ӵ���
    };

    BYTE byResult;          // �������
    BYTE byRefreshType;     // ˢ�·�ʽ
    DWORD dwCardPoint;      // ʣ�����

	// ��������ADDHERE

    DWORD dwActionCode;     // �����루�ش���

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ���֮��
// EPRO_EXCHANGECARDPOINT
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ֪ͨ�˺ŷ�������ĳ���˺��˳���Ϸ����Ҫֹͣ�Ƿ�
DECLARE_MSG(SAccountLogoutBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_ACCOUNT_LOGOUT)
struct SQAccountLogoutMsg :
    public SAccountLogoutBaseMsg
{
    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};

struct SAAccountLogoutMsg :
    public SAccountLogoutBaseMsg
{
    enum
    {
        RET_SUCCESS,
        RET_INVALID_ACCOUNT,        // ��Ч���˺�
        RET_INVALID_PASSWORD,       // ��Ч������
        RET_BADINFO,
        LOGOUT_ERROR_NOTLOGIN,      // Player Logout without login ,some thing wrong happend 
        LOGOUT_ERROR_NORIGHT,
        LOGOUT_ERROR_CATCHERROR,    // 
        LOGOUT_ERROR_NOSESSIONID,
        LOGOUT_ERROR_UNHANDLED
    };

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ֪ͨ�˺ŷ�������ĳ���˺��˳���Ϸ����Ҫֹͣ�Ƿ�
DECLARE_MSG(SAccountHangupBaseMsg, SAccountBaseMsg, SAccountBaseMsg::EPRO_ACCOUNT_HANGUP)
struct SQAccountHangupMsg :
    public SAccountHangupBaseMsg
{
    BYTE byPoint;
    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};
//---------------------------------------------------------------------------------------------



//=============================================================================================
// ����һ���˺�
DECLARE_MSG(SAddAccountBaseMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_PLAYER_ADD_ACCOUNT)
struct SQAddAccountMsg : 
    public SAddAccountBaseMsg
{
    struct SNewPlayerInfo
    {
        char szName     [32];
        int  iSex ;
        int  iAge;
        char szAddr1 [64];
        char szCity  [32];
        char szProvince [32];
        char szCountry [32];
    };

    SNewPlayerInfo PlayerData;

    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};

struct SAAddAccountMsg :
    public SAddAccountBaseMsg
{
    enum ADD_ACCOUNT_RET
    {
        ADD_ACCOUNT_SUCESS,
        ADD_ACCOUNT_ERROR_ACCOUNTEXISTS,
        ADD_ACCOUNT_ERROR_NAMEEXISTS,
        ADD_ACCOUNT_BAD_INFO
    };

    BYTE byResult;
};
//---------------------------------------------------------------------------------------------

//
enum ADD_CARDPOINT_RET
{
    ADD_CARDPOINT_SUCCESS,
    ADD_CARDPOINT_ERROR_NOACCOUNT,
    ADD_CARDPOINT_ERROR_INGAME,//the player is in the game ,cannot add card point
    ADD_CARDPOINT_ERROR_BAD_INFO,
    ADD_CARDPOINT_UNHANDLED
};


enum PRODUCT_REGIST_RET // Player Regist product return value
{
    PROREG_SUCCESS,
    PROREG_ERROR_NOPRODUCT,//the product
    PROREG_ERROR_TIMEAHEAD,//the product will be registed in the furture
    PROREG_BAD_INFO
};
//

//////////////////////////////////////////////
// Ԫ������ˢ��
//#include "MoneyPointDef.h"
DECLARE_MSG(SLRefreshMoneyPointMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_REFRESH_MONEY_PIONT)
struct SQLRefreshMoneyPointMsg : public SLRefreshMoneyPointMsg
{
	//SMoneyPointNotify mpn;
	enum { RT_QUEST, RT_REPLACE };
	DWORD dwRefType;
	DWORD dwMoneyPoint;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SALRefreshMoneyPointMsg : public SLRefreshMoneyPointMsg
{
	enum { R_INVALID_ACCOUNT, R_NOTENOUGH_POINT, R_OK, R_COMPLETE}; 
	DWORD	dwRet;
	DWORD dwMoneyPoint;		

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

DECLARE_MSG(SQueryEncouragementMsg,SAccountBaseMsg,SAccountBaseMsg::EPRO_QUERY_ENCOURAGEMENT)

struct SQQueryEncouragement :public SQueryEncouragementMsg
{
    char m_szName[CONST_USERNAME];
    char m_szID[20];
};

struct SAQueryEncouragement:public SQueryEncouragementMsg
{
	char m_szName[CONST_USERNAME];
	BYTE m_iRet;
	enum
	{
        RET_INVALIDCARD = 2,
	    RET_ISUSEED =3,
	    RET_WEEKOK = 101,		//����OK
	    RET_MONTH = 102 //����OK
	};
};

//////////////////////////////////////////////

DECLARE_MSG( SUpdateTimeLimit, SAccountBaseMsg, SAccountBaseMsg::EPRO_UPDATE_TIMELIMIT )

struct SQUpdateTimeLimit : public SUpdateTimeLimit
{
    // str key
    char streamBuffer[256];
};
