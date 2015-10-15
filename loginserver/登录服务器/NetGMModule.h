//


#pragma once 
#include "networkmodule\GMModuleMsg.h"
#include <map>
#include <string>
#include "NetWorkModule/NetProviderModule/NetProviderModuleAdv.h"
#include "..\pub\ConstValue.h"


struct SGMContextInfo 
{
    WORD m_wGMLevel;
    WORD m_wOprParam[5]; //����������Ŀǰ0���Ƿ���ղ鿴������Ϣ
    SGMContextInfo()
    {
        m_wGMLevel = 0;
        ZeroMemory(m_wOprParam,sizeof(WORD)*5);
    };
    ~SGMContextInfo()
    {
    };

    // �˺ţ�����
    std::string account;
    std::string password;
};




struct SGMInfo
{
    WORD wGMLevel;
    //more to be add in the future;
};
// Զ����Բ���

struct SSaveInfo
{
    QWORD qwSerialNo;
    SGMMMsg * pGMMMsg;
    WORD wType;
};

//Զ�̲�������Ϣ����Բ���
class CDistributedOPR
{
    std::map<QWORD,SSaveInfo*> m_StoreOpr; //map<������Ϣ��QWORD,�洢����Ϣ�ṹ>
public:
    BOOL GetBuffer(QWORD qwSerialNo,SGMMMsg *pGMMMsg,WORD &wType);
    void PushBuffer(SGMMMsg * pGMMMsg,QWORD qwSerialNo,WORD wMessageType);
    CDistributedOPR();
    ~CDistributedOPR();
};

class CNetGMModule
{

    BOOL CheckGMLevel(DNID dnidClient,int iCheckLevel);
public:
    CNetProviderModuleAdv      m_GMServer;
    std::map<DNID, SGMContextInfo> m_GMContextMap;  // ������ά����GMContextӳ���
    bool OnDispatchGM( DNID dnidClient,LINKSTATUS enumStatus, LPVOID data, LPVOID attachment );
    int SendGMMessage(DNID dnidClient, LPVOID pMsg, WORD wSize);

    int OnGMLogin(DNID dnidClient,SGMMMsg * pMsg);

    int GMLogout(LPCSTR szAccount,LPCSTR szPassword);
    SGMContextInfo &GetLinkContext(DNID dnidClient);
    BOOL RemoveLinkContext(DNID dnidClient);

    void SendDistributeMessage(SGMMMsg * pMsg,WORD wSize,WORD wType);

    BOOL SetGMLinkContext(DNID dnidClient, LPCSTR szAccount, LPCSTR szPassword,WORD wGMLevel);

    BOOL SetGMLinkContext(DNID dnid,SGMContextInfo &LinkContextInfo);

    BOOL CheckOnline(DNID dnidClient,LPCSTR szKey ,WORD wType);//The dnid is the dnid for the GMTool,not for the client on the server

	void ProcInfo();
	void SendInfo();

	int Execution();
    int OnGMMessage(DNID dnidClient,SGMMMsg * pGMMsg);
    int OnChatCheckMessage(SAGMCheckChatGMMsg * pGMMsg);
    void DoGMLogin( struct SAGMCheckAccountMsg *pMsg );
    void OnRecvGMMsgFromGameServer( DNID dnidClient, SGMMMsg *pGMMMsg, size_t size );
    int Destory();
    
	BOOL Init(LPCSTR port);
    CNetGMModule();
    ~CNetGMModule();

    BOOL GetAddrByDnid(DNID dnidClient, char *szIP , DWORD size);

	CPublicInfoTab m_InfoTab;			//�������ϴ洢�Ĺ�����Ϣ

	CPublicInfoTab m_buffInfoTab;		//����ʾ����Ϣ�б�

};


typedef enum 
{
    GM_LEVEL_1,
    GM_LEVEL_2,
    GM_LEVEL_3,
    GM_LEVEL_4,
    GM_LEVEL_5
}GMLEVEL;
typedef enum
{
    GM_ACT_MOVEPLAYER,
    GM_ACT_CHECKPLAYER,
    GM_ACT_CHANGEPLAYER
}GM_ACTION;
struct SGMOperation
{
    GMLEVEL GMLevel;
    GM_ACTION GMAction;

};

class CGMLevelChecker
{
    CGMLevelChecker();
    ~CGMLevelChecker();
    int CheckLevel(GM_ACTION Action,WORD Level);
};

//-----------------------------------------------------------------------
//Add By Lovelonely
//-----------------------------------------------------------------------
struct SGMIP
{
	char m_szIP[17];
};

struct SPlayerName
{
	char szName[CONST_USERNAME];
};

//-----------------------------------------------------------------------
class CGM
{
public:
	CGM();

	DNID						m_dnidClient;
	dwt::stringkey<char[CONST_USERNAME]>	m_szID;
	void						ReturnPing();

	BOOL						m_bIsVisible;
	int							m_iPing;
	int							m_iNum;
	int							m_iMaxPing;
	int							m_iMinPing;

	int							m_iTimeSkip;

};
//-----------------------------------------------------------------------
class CGMManager
{
public:
	CGMManager();
	~CGMManager();

	void					OnGMLog(LPCSTR szID,DNID dnID);				//��gm��gm�����ߵ�½��ʱ�����m_GMMap	
	void					OnGMOut(LPCSTR szID);

	CGM *					FindGM(LPCSTR szID);

	BOOL					CheckGMIP(DNID dnidClient, BOOL isGM);

	BOOL					LoadGMIP(void);
	BOOL					LoadGaolList(void);
	BOOL					SaveGaolList(void);

	std::list<SGMIP>		m_listGMIP;

    std::map<std::string,CGM>	m_GMMap;						//��������ά����һ��gm�� ���������й������ߵ�gm
	
    //std::map<std::string,CGM>	m_OnlineGMMap;					//�н�ɫ���ߵ�gm�б�

	std::list<SPlayerName>					m_listGaol;						//����������б�������ϵͳ�Զ�ץ��ȥ�ģ�
};
//-----------------------------------------------------------------------


