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
    WORD m_wOprParam[5]; //操作参数，目前0是是否接收查看聊天信息
    SGMContextInfo()
    {
        m_wGMLevel = 0;
        ZeroMemory(m_wOprParam,sizeof(WORD)*5);
    };
    ~SGMContextInfo()
    {
    };

    // 账号，密码
    std::string account;
    std::string password;
};




struct SGMInfo
{
    WORD wGMLevel;
    //more to be add in the future;
};
// 远程配对操作

struct SSaveInfo
{
    QWORD qwSerialNo;
    SGMMMsg * pGMMMsg;
    WORD wType;
};

//远程操作的消息，配对操作
class CDistributedOPR
{
    std::map<QWORD,SSaveInfo*> m_StoreOpr; //map<发送消息的QWORD,存储的消息结构>
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
    std::map<DNID, SGMContextInfo> m_GMContextMap;  // 服务器维护的GMContext映射表
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

	CPublicInfoTab m_InfoTab;			//服务器上存储的公告信息

	CPublicInfoTab m_buffInfoTab;		//待显示的信息列表

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

	void					OnGMLog(LPCSTR szID,DNID dnID);				//有gm用gm管理工具登陆的时候，填充m_GMMap	
	void					OnGMOut(LPCSTR szID);

	CGM *					FindGM(LPCSTR szID);

	BOOL					CheckGMIP(DNID dnidClient, BOOL isGM);

	BOOL					LoadGMIP(void);
	BOOL					LoadGaolList(void);
	BOOL					SaveGaolList(void);

	std::list<SGMIP>		m_listGMIP;

    std::map<std::string,CGM>	m_GMMap;						//服务器端维护的一个gm表 ，包括所有工具在线的gm
	
    //std::map<std::string,CGM>	m_OnlineGMMap;					//有角色在线的gm列表

	std::list<SPlayerName>					m_listGaol;						//监狱内玩家列表（不包括系统自动抓进去的）
};
//-----------------------------------------------------------------------


