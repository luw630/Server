//////////////////////////////////////////
//FileName		    :   GMModuleMsg.h   // 
//FileAuthor		:	luou            //
//FileCreateDate	:	15:09 2004-2-5  //
//FileDescription	:	GMģ�鴦�����Ϣ//
//////////////////////////////////////////
#pragma once

#include "NetModule.h"
#include "chattypedef.h"
#include "itemtypedef.h"
#include "playertypedef.h"
#include "../pub/ConstValue.h"





///////////////////////////////////////////////////////////////////////////////////////////////
// GM�����Ϣ��
///////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================================
DECLARE_MSG_MAP(SGMMMsg, SMessage, SMessage::EPRO_GMM_MESSAGE)
//{{AFX
EPRO_GM_TALKMASK,		//��ĳ����
EPRO_GM_TALKUNMASK,
EPRO_GM_MOVETOHIM,
EPRO_GM_MOVEBYME,
EPRO_GM_MOVETOBORN,
EPRO_GM_SENDTALK,		//���͹�����Ϣ
EPRO_GM_ACCOUNTMASK,
EPRO_GM_ACCOUNTUNMASK,
EPRO_GM_CHECKPLAYER,	//��ĳ�˵ı�����װ��

EPRO_GM_CHANGENAME,
EPRO_GM_CHANGEMORE,
EPRO_CHECK_SB_INFO,		//��ĳ�˵����ϣ������ȼ�����ͼλ�ã�����ʱ�䡣������
EPRO_CHECK_ONLINE,		//�����������
EPRO_GM_CHANGEPASS,		//���������
EPRO_GM_BLOCKACCOUNT,
EPRO_GM_GETONLINETIME,
EPRO_GM_LEVELDEF,		//GM�ȼ�����

EPRO_GM_CHECKPLAYER_DB,
EPRO_GM_CHECKWAREHOUSE, // GM ��ѯ��Ҳֿ����Ϣ
EPRO_GM_LOGIN,			// GM �����¼����������Ϣ
EPRO_GM_LOGOUT,			// GM �ǳ���¼����������Ϣ
EPRO_GM_CHECKONLINE,	// GM �������Ƿ�����
EPRO_UPDATE_SB_INFO,	// GM �����������

EPRO_GM_PROBLEM,		//����������
EPRO_GM_GETREQUEST,		//gm���߶����ȡ�ȴ������������

EPRO_GM_SAYGLOBAL,		//GM��ǧ�ﴫ��
EPRO_GM_CHECKCHAT,
EPRO_GM_WISPER,			//gm�����֮���������Ϣ
EPRO_GM_CLOSE,			//�طż���
EPRO_GM_CHECKSCORE,		//��ѯ10�����а�
EPRO_GM_CUT,			//gm���������
EPRO_GM_PUBLICINFO,		//��������Ϣ
EPRO_GM_CHANGEPUBINFO,	//���Ĺ�����Ϣ
EPRO_GM_CHECKONLINEGM,  //��Ҳ�ѯ����gm
EPRO_GM_GETPING,		//ȡ���pingֵ
EPRO_GM_REFPLAYERNUM,	//ˢ���������~~
EPRO_GM_TALKMASKSRV,	//���Բ��� ��Ϊ��������������� ��������һ����������ʹ�õĽ�����Ϣ
EPRO_GM_GETMASKNAME,	//��ȡ��������~~
EPRO_GM_GETNAMELIST,	//��ȡ��ɫ�����б�
EPRO_GM_MOVETOSAFEPOINT,//������ҵĽ�ɫ��ָ�������İ�ȫ�㣨����ͳ����ڹ��߶�ָ����
EPRO_GM_VISIBLE,		//�Ƿ���������GM
EPRO_GM_CMD = 42,       // GM����֪ͨ�ͻ���ִ�в���
EPRO_GM_CHECKGMIDINF,   //��֤GM�����Ϣ

//}}AFX
END_MSG_MAP_WITH_ROTOC_GMLEVEL()


//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMCMD,SGMMMsg,SGMMMsg::EPRO_GM_CMD)
struct SQEXECGMCMD : public SBaseGMCMD
{
    char streamData[4096];
};

struct SAEXECGMCMD : public SBaseGMCMD
{
    char streamData[4096];
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMTalkMsg,SGMMMsg,SGMMMsg::EPRO_GM_SENDTALK)
struct SQGMTalkMsg : public SBaseGMTalkMsg
{
    char szTalkMsg[MAX_CHAT_LEN];
};
struct SAGMTalkMsg : public SBaseGMTalkMsg
{
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseMovetoBornMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOBORN)
struct SQMovetoBornMsg : public SBaseMovetoBornMsg
{
    char szName[CONST_USERNAME];
};
struct SAMovetoBornMsg : public SBaseMovetoBornMsg
{
    char szName[CONST_USERNAME];
    BYTE byResult; 
    // 1 ,�ɹ�
    // 0 ,�����ߣ�ʧ��
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseCheckPLOnlineMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKONLINE)
struct SQCheckPLOnlineMsg : public SBaseCheckPLOnlineMsg
{
    char szName[CONST_USERNAME];
    BYTE bType; // Check type , 0 Name ,1 Account

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SACheckPLOnlineMsg : public SBaseCheckPLOnlineMsg
{
    BYTE bOnline; // 1 , online 0 ,offline
};



//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseCheckWareHouseMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKWAREHOUSE)
struct SQCheckWareHouseMsg : public SBaseCheckWareHouseMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
struct SACheckWareHouseMsg : public SBaseCheckWareHouseMsg
{  
    WORD wResult; // 
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
DECLARE_MSG(SBaseCheckPlayerDatabaseMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKPLAYER_DB)
struct SQCheckPlayerDatabaseMsg : public SBaseCheckPlayerDatabaseMsg
{
    char szName[CONST_USERNAME];
};
struct SACheckPlayerDatabaseMsg : public SBaseCheckPlayerDatabaseMsg
{  
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;
    char szIP[17];

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};


//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMLoginMsg,SGMMMsg,SGMMMsg::EPRO_GM_LOGIN)
struct SQGMLoginMsg : public SBaseGMLoginMsg
{
    // �˺ţ�����
	char streamData[ MAX_ACCAPASS ];
};
struct SAGMLoginMsg : public SBaseGMLoginMsg
{
    WORD wResult;
    WORD wGMLevel;
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
        ERC_CHECK_CHAT,                 // �鿴���������Ϣ
		ERC_IP_ERROR,					// ip��֤ʧ��
	};

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

//=============================================================================================
DECLARE_MSG(SGMLogoutMsg, SGMMMsg, SGMMMsg::EPRO_GM_LOGOUT)
// ����������������ӣ��򵥰棩�����������û��汾�ţ��˺ţ�����
struct SQGMLogoutMsg : public SGMLogoutMsg  // �޷�������
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
struct SAGMLogoutMsg : public SGMLogoutMsg  // �޷�������
{
    BYTE byResult;
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
};

//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGMLevelDef,SGMMMsg,SGMMMsg::EPRO_GM_LEVELDEF)
struct SAGBLevelDef : public  SBaseGMLevelDef
{

};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseGetOnlineTimeMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETONLINETIME)
struct SAGetOnlineTimeMsg : public SBaseGetOnlineTimeMsg 
{
    int iOnlineTime;
    WORD wResult;
    enum 
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
struct SQGetOnlineTimeMsg : public SBaseGetOnlineTimeMsg
{
    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseBlockAccountMsg,SGMMMsg,SGMMMsg::EPRO_GM_BLOCKACCOUNT)
struct SABlockAccountMsg : public SBaseBlockAccountMsg
{
    DWORD dwBlockDuration;
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_NOACCOUNT,
        RT_ERROR
    };

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
struct SQBlockAccountMsg : public SBaseBlockAccountMsg
{
    DWORD dwBlockDuration;

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};
//---------------------------------------------------------------------------------------------
DECLARE_MSG(SBaseMoveTohimMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOHIM)
struct SAMoveTohimMsg : public SBaseMoveTohimMsg
{
    WORD wResult;
};

struct SQMoveTohimMsg : public SBaseMoveTohimMsg
{
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseCheckOnlineMsg,SGMMMsg,SGMMMsg::EPRO_CHECK_ONLINE)
struct SACheckOnlineMsg : public SBaseCheckOnlineMsg
{
    DWORD dwOnlineCount;
    DWORD dwMaxCount;

};
struct SQCheckOnlineMsg : public SBaseCheckOnlineMsg
{
};

DECLARE_MSG(SBaseTalkMaskGMMsg, SGMMMsg, SGMMMsg::EPRO_GM_TALKMASK)
struct SATalkMaskGMMsg : public SBaseTalkMaskGMMsg
{
    char szName[CONST_USERNAME];
    DWORD dwTime;
    WORD wResult; // 0 ʧ�ܣ�����Ҳ�����
    // 1 �ɹ�
    // 2 ʧ�ܣ�������Ѿ�������
};

struct SQTalkMaskGMMsg : public SBaseTalkMaskGMMsg
{
    char szName[CONST_USERNAME]; // �����Ե���ҵ�����
    DWORD dwTime;      // �����Ե�ʱ��
};

DECLARE_MSG(SBaseTalkUnMaskGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_TALKUNMASK)
struct SATalkUnMaskGMMsg : public SBaseTalkUnMaskGMMsg
{
    char szName[CONST_USERNAME];
    WORD wResult; 
    // 1 �ɹ�
    // 2 ʧ��
};

struct SQTalkUnMaskGMMsg : public SBaseTalkUnMaskGMMsg
{  
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseMovebyMeGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVEBYME)
struct SAMovebyMeGMMsg : public SBaseMovebyMeGMMsg
{
    WORD wResult;  
};
struct SQMovebyMeGMMsg : public SBaseMovebyMeGMMsg
{
    char szName[CONST_USERNAME];
};

DECLARE_MSG(SBaseCheckSBInfo,SGMMMsg,SGMMMsg::EPRO_CHECK_SB_INFO)
struct SQCheckSBInfoGMMsg : public SBaseCheckSBInfo
{
    char szName[CONST_USERNAME];
};
struct SACheckSBInfoGMMsg : public SBaseCheckSBInfo
{
    WORD wResult;
    DWORD dwStaticID;
    SFixBaseData PlayerData;
    SFixPackage  PlayerPackage;
    SPlayerTasks PlayerTasks;
    char cszIP[17];
};

DECLARE_MSG(SBaseUpdateSBInfo,SGMMMsg,SGMMMsg::EPRO_UPDATE_SB_INFO)
struct SQUpdateSBInfoMsg : public SBaseUpdateSBInfo
{
    DWORD dwStaticID;
    SFixBaseData PlayerData;
};
struct SAUpdateSBInfoMsg : public SBaseUpdateSBInfo
{
    WORD wResult; // 0 ʧ�� �� 1 �ɹ�
	char szName[CONST_USERNAME];
};

//-----------------------------------------------------------------------
//Add by LoveLonely 20040302
//-----------------------------------------------------------------------




DECLARE_MSG(SBaseMoveToSafePointGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_MOVETOSAFEPOINT)

struct SQMoveToSafePointGMMsg :public SBaseMoveToSafePointGMMsg
{
	char szName[CONST_USERNAME];
	int iRegion;
	int iX;
	int iY;
};

struct SAMoveToSafePointGMMsg :public SBaseMoveToSafePointGMMsg
{
	char szName[CONST_USERNAME];

	WORD wRet;
	enum
	{
		RET_SUCCESS, //�ɹ�
		RET_FAIL,	 //ʧ��	
	};
};

DECLARE_MSG(SBaseGetMaskNameGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETMASKNAME)

struct SQGetMaskNameGMMsg :public SBaseGetMaskNameGMMsg
{
	WORD wPara;
	enum
	{
		PA_GAOL,
		PA_TALKMASK,
	};
};

struct SAGetMaskNameGMMsg :public SBaseGetMaskNameGMMsg
{
	char szName[20];

	WORD wPara;
	enum
	{
		PA_BEGIN,
		PA_SEND,
		PA_END,
	};

	WORD wResult;
	enum
	{
		RT_GAOL,
		RT_TALKMASK,
	};

};

DECLARE_MSG(SBaseTalkMaskSrvGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_TALKMASKSRV)

struct SQTalkMaskSrvGMMsg :public SBaseTalkMaskSrvGMMsg
{
	DWORD	dwTime;
	char	szName[CONST_USERNAME];

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};//��½�������������������

DECLARE_MSG(SBaseRefPlayerNumGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_REFPLAYERNUM)

struct SARefPlayerNumGMMsg :public SBaseRefPlayerNumGMMsg
{
	int iPlayerNum;
	int iPlayerNotOnline;
	int iPlayerAuto;
};//ֻ�Ƿ������������ߵġ�so ֻ�� A ��Ϣ

DECLARE_MSG(SBaseGetPingGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETPING)

struct SQGetPingGMMsg :public SBaseGetPingGMMsg
{
	int  iPing;
    DWORD ip32;
    QWORD mac64;
	WORD wPara;
	enum
	{
		PA_GET,
		PA_RETURN,
	};
};

struct SAGetPingGMMsg :public SBaseGetPingGMMsg
{
	int  iPing;
    DWORD ip32;
    QWORD mac64;
};

DECLARE_MSG(SBaseCheckOnlineGMGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKONLINEGM)

struct SQCheckOnlineGMGMMsg :public SBaseCheckOnlineGMGMMsg
{
	char szName[CONST_USERNAME];

	char szGM[10][CONST_USERNAME];

	WORD wPara;
	enum
	{
		PA_GET,
		PA_RETURN,
	};
};

DECLARE_MSG(SBaseChangePubInfoGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHANGEPUBINFO)

struct SQChangePubInfoGMMsg :public SBaseChangePubInfoGMMsg
{
	char szInfo[10][MAXPUBLICINFO];

	WORD wPara;
	enum
	{
		PA_GET,
		PA_CHANGE,
	};
};

struct SAChangePubInfoGMMsg :public SBaseChangePubInfoGMMsg
{
	char szInfo[10][MAXPUBLICINFO];

	WORD wRet;
	enum
	{
		RT_RETURN,
		RT_SUCCESS,
		RT_ERROR,
	};
};

DECLARE_MSG(SBasePublicInfoGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_PUBLICINFO)
struct SQPublicInfoGMMsg :public SBasePublicInfoGMMsg
{
	int GetMySize(){return sizeof(SQPublicInfoGMMsg)-(128-byLength);};

	BYTE	byLength;

	long	lClr;					//��ɫ
	int		nTime;					//���ʱ��

	WORD	wType;
	enum
	{
		TP_CHANNEL,					//����Ƶ����ʾ
		TP_ROLL,					//������ʾ
	};

	char	szSay[128];				//��
};

struct SAPublicInfoGMMsg :public SBasePublicInfoGMMsg
{
	int GetMySize(){return sizeof(SQPublicInfoGMMsg)-(128-byLength);};

	BYTE	byLength;

	WORD	wType;

	enum
	{
		TP_CHANNEL,					//����Ƶ����ʾ
		TP_ROLL,					//������ʾ
	};

	WORD wResult;
	enum
	{
		RT_SUCCESS,
		RT_ERROR,
	};

	char	szSay[128];				//����
};

DECLARE_MSG(SBaseCutGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CUT)
struct SQCutGMMsg  :public SBaseCutGMMsg
{
	char szGM[CONST_USERNAME];
	char szName[CONST_USERNAME];
};

struct SACutGMMsg  :public SBaseCutGMMsg
{
	char szName[CONST_USERNAME];

	WORD wResult;
	enum
	{
		RT_SUCCESS,
		RT_ERROR,
	};
};
//
//DECLARE_MSG(SBaseCheckScoreGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKSCORE)
//struct SQCheckScoreGMMsg  :public SBaseCheckScoreGMMsg
//{
//	char szGM[CONST_USERNAME];
//};
//
//struct SACheckScoreGMMsg  :public SBaseCheckScoreGMMsg
//{
//	SScoreTable s_ScoreTab;
//};



DECLARE_MSG(SBaseCloseGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CLOSE)
struct SQCloseGMMsg		: public SBaseCloseGMMsg
{
	char szName[CONST_USERNAME];
	char szGM[CONST_USERNAME];
	WORD wPara;
	enum
	{
		PARA_SETFREE,
		PARA_CLOSE,
	};
};

struct SACloseGMMsg		: public SBaseCloseGMMsg
{
	char szName[CONST_USERNAME];
	char szGM[CONST_USERNAME];
	WORD wResult;
	enum
	{
		RT_CLOSESUC,
		RT_SETFREESUC,
		RT_ERROR,
	};

};


DECLARE_MSG(SBaseSayGlobalGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_SAYGLOBAL)
struct SQSayGlobalGMMsg  : public SBaseSayGlobalGMMsg
{
	char _szSay[250];
	char szName[CONST_USERNAME];
};

struct SASayGlobalGMMsg	 : public SBaseSayGlobalGMMsg
{
	char _szSay[250];
	char szName[CONST_USERNAME];
	
	WORD wResult;
	enum
	{
		RT_SUC,
		RT_ERROR,	
	};

};

DECLARE_MSG(SBaseWisperGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_WISPER)
struct SQWisperGMMsg	:public SBaseWisperGMMsg
{
	SQWisperGMMsg()
	{
		dwConsumePoint = 0;
	}
	char szGM[CONST_USERNAME];
	char szPlayer[CONST_USERNAME];
	char szSay[MAX_CHAT_LEN];
	DWORD dwConsumePoint; //���ѻ���

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};

struct SAWisperGMMsg	:public SBaseWisperGMMsg
{
   SAWisperGMMsg()
	{
	dwConsumePoint = 0;
	}
	char szGM[CONST_USERNAME];
	char szPlayer[CONST_USERNAME];
	char szSay[MAX_CHAT_LEN];
	DWORD dwConsumePoint; //���ѻ���
	WORD wResult;
	enum
	{
		RT_ERROR,	
	};

    // �˺�
	char streamData[ MAX_ACCOUNT ];
};


DECLARE_MSG(SBaseChangePassGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHANGEPASS)
struct SQChangePassGMMsg : public SBaseChangePassGMMsg
{
    // �˺ţ����룬������
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};
struct SAChangePassGMMsg : public SBaseChangePassGMMsg
{
    WORD wResult;
    enum
    {
        RT_SUCCESS,
        RT_WRONGPASS,
        RT_ERROR    
    };

    // �˺ţ����룬������
	char streamData[ MAX_ACCOUNT + MAX_PASSWORD*2 ];
};

DECLARE_MSG(SBasePlayerProblemGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_PROBLEM)
struct SQPlayerProblemGMMsg :public SBasePlayerProblemGMMsg
{
	char szName[CONST_USERNAME];
	char szRequest[MAX_CHAT_LEN];
	

	bool bIsRequest;		//trueΪ�״�����falseΪgm���ܴ����ĶԻ�
	char szGMID[CONST_USERNAME];		//gmID �����ʱ��Ϊ��
};
struct SAPlayerProblemGMMsg	:public SBasePlayerProblemGMMsg
{
	char szName[CONST_USERNAME];
	char szGMID[CONST_USERNAME];
	char szTalkText[MAX_CHAT_LEN];
	

	WORD wResult;
	enum
	{
		RT_TAKEON,			//���ܴ���
		RT_PASSUP,			//�ܾ�����
		RT_NORMAL,			//����ing
		RT_END,				//��������
		RT_NOGM,			//��gm����
		RT_DROP,			//gm����
	};

};

DECLARE_MSG(SBaseGetRequestGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_GETREQUEST)
struct SQGetRequestGMMsg :public SBaseGetRequestGMMsg
{
	char szGMID[CONST_USERNAME];		//��������gmID
};

struct SAGetRequestGMMsg :public SBaseGetRequestGMMsg
{
	char szName[CONST_USERNAME];	
	char szRequest[MAX_CHAT_LEN];

	WORD wResult;
	enum
	{
		RT_SUCCESS,			//�ɹ�
		RT_EMPTY,			//�ȴ�����Ϊ��
	};
};
//-----------------------------------------------------------------------
//=============================================================================================
DECLARE_MSG(SBaseCheckChatGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKCHAT)
struct SQGMCheckChatGMMsg : public SBaseCheckChatGMMsg
{
    BYTE byOpr ; //0 ֹͣ���գ�1��ʼ����
};
struct SAGMCheckChatGMMsg : public SBaseCheckChatGMMsg
{
    WORD wChatType;       // ˵��������
    WORD wChatPara;       // ˵���Ĳ�������ϵͳ��Ϣ���ࣩ
    WORD wChatLen;        // ������Ϣ����
    WORD wGMLevel;        // GM�ȼ�
    char szNameTalker[CONST_USERNAME];// ˵������
    char szNameRecver[CONST_USERNAME];// ���ܵ��ˣ�����Ϊ�գ�
    char szMessage[MAX_CHAT_LEN];   // ˵��������
};



DECLARE_MSG( SBaseVisibleGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_VISIBLE )
struct SQVisibleMsg : public SBaseVisibleGMMsg
{
	BYTE byOerate;	// �Ƿ���������GM
	char szName[CONST_USERNAME];	// GM����
};

struct SAVisibleMsg : public SBaseVisibleGMMsg
{
	BYTE byRet;		// ����ֵ
	char szName[CONST_USERNAME];	// GM����
};

DECLARE_MSG( SBaseCheckGMInfGMMsg,SGMMMsg,SGMMMsg::EPRO_GM_CHECKGMIDINF )
struct SACheckGMInfMsg : public SBaseCheckGMInfGMMsg            // ��KFDҪGMID��Ϣ
{
    DWORD dwRandNumber;  // �����
};

struct SQCheckGMInfMsg : public SBaseCheckGMInfGMMsg            // ����KFD������Ϣ
{
    char szName[CONST_USERNAME];	// GM����
    char streamData[1024];	// ����û�����ַ���
};
