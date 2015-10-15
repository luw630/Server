#pragma once
#include <memory>
#include "playermanager.h"
#include "regionmanager.h"
#include "dchatmanager.h"
#include "teammanager.h"
#include "unionmanage.h"
#include "factionmanager.h"
#include "���������/���������/NetGMModule.h"
#include "networkmodule/chatMsgs.h"
#include "StackWalker.h"
#include "SceneMap.h"
#include "Transformers.h"
#include "ArenaMap.h"
class   CPlayer;
class CLuckyTimeSystem;
struct SQSynSimPlayerMsg;

//typedef std::hash_map<WORD, BossDeadData>BossDeadDataTable;		//<����ID����>
typedef std::vector<BossDeadData>BossDeadDataTable;		//<����ID����>
typedef std::vector<ActivityNotice>ActivityNoticeTable;		//<�ճ��>
struct GLoblessInfoSTR
{
	std::string name;
	std::string ItemName;
};
class CGameWorld : 
	public CRegionManager, public CPlayerManager, public CDChatManager
{
//-----------------------------------------------------------------
// ˽�к���
typedef std::list<GLoblessInfoSTR> GLOBALBLESSINFO;
public:
    int GetAccountLimit( CPlayer *pPlayer );                             // ��ȡ�˺�Ȩ��
//-----------------------------------------------------------------
// һЩ�ӿں���
public:
	BOOL OnDispatch(DNID dnidClient, struct SMessage *pMsg, CPlayer *pPlayer);

    // �����ڼ����ݴ�����ص���Ϣ����
    BOOL OnDispathDTM(SMessage *pMsg, WORD wMsgSize, CPlayer *pPlayer);
    BOOL OnDispathDTM(SMessage *pMsg, WORD wMsgSize);

    // �ͻ������ݴ�����ص���Ϣ����
	BOOL DispatchSysMessage(DNID dnidClient, struct SSysBaseMsg *pMsg, CPlayer *pPlayer); // �������Ϣ�ɷ�����
    BOOL DispatchChatMessage(DNID dnidClient, struct SChatBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchRegionMessage(DNID dnidClient, struct SRegionBaseMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchGameMngMessage(DNID dnidClient, struct SQGameMngMsg *pMsg, CPlayer *pPlayer);
    BOOL DispatchMenuMessage(DNID dnidClient, struct SQMenuMsg * pMsg, CPlayer *pPlayer);
    BOOL DispatchTongMessage(DNID dnidClient, struct STongBaseMsg *pMsg, CPlayer *pPlayer);
	BOOL DispatchCardMessage(DNID dnidClient, struct SPointModifyMsg *pMsg, CPlayer *pPlayer);
	BOOL DispatchMailMessage(DNID dnidClient, struct SMailBaseMsg *pMsg);
	BOOL DispatchPhoneMessage(DNID dnidClient, struct SBasePhoneMsg *pMsg);
    BOOL DispatchChatServerMessage(struct SMessage * pMsg);
	BOOL DispatchFacBBSMessage( DNID dnidClient, struct SFactionBBSMsg *pMsg, CPlayer *pPlayer );
    BOOL DispatchSceneMessage(DNID dnidClient, struct SSceneMsg *pMsg, CPlayer *pPlayer); //�ؿ�������Ϣ
	BOOL DispatchTransformersMessage(DNID dnidClient, struct STransformersMsgs *pMsg, CPlayer *pPlayer); //������Ϣ
	BOOL DispatchDyArenaMessage(DNID dnidClient, struct SArenaMsg *pMsg, CPlayer *pPlayer); //��������Ϣ
	BOOL DispatchDyArenaMessage(DNID dnidClient, SSGArenaMsg *pMsg, CPlayer *pPlayer); //����������
#ifdef GMMODULEON
    BOOL DispatchGMModuleMessage(DNID dnidClient,struct SGMMMsg * pMsg,CPlayer *pPlayer,BOOL bDistribute);
#endif

//-----------------------------------------------------------------
// һЩ���幦�ܺ���
    
public:
    BOOL SetPlayerData(DNID dnidClient, DWORD GID, SFixData *Data, SPlayerTempData *pTempData );
    BOOL CreateNewPlayer( DNID dnidClient, LPCSTR acc16, DWORD GID, DWORD limitedState, DWORD online, DWORD offline, QWORD puid );
	BOOL PutPlayerIntoRegion(LPIObject pPlayer, WORD wNewRegionID, WORD wStartX, WORD wStartY, DWORD dwRegionGID = 0, bool bStoreMessage = true);
    BOOL RebindPlayerToNewRegionServer(struct SAResolutionDestRegionMsg *pMsg);
    BOOL RefreshPlayerCardPoint(struct SARefreshCardPointMsg *pMsg);
    const SQSynSimPlayerMsg * GetRegisterPlayerToORBMsg(DWORD dwGID, WORD wServerID);

    BOOL QuestJoinFaction(DNID dnidClient, DWORD GID, CPlayer *pPlayer);
    BOOL LookOtherPlayerEquipment(CPlayer *pSrcPlayer, CPlayer *pDestPlayer, bool b_notice = true);

    BOOL DispatchRelationMessage(DNID dnidClient,struct SRelationBaseMsg * pMsg, CPlayer *pPlayer);

	// Ԫ�����ֵ�ˢ��
	BOOL RefreshPlayerMoneyPoint(struct SARefreshMoneyPointMsg *pMsg);

	// ����һ��㿨
	BOOL NotifyMCInfo(struct SExchangeMCMsg *pMsg);

	// EQVPoint֪ͨ
	BOOL NotifyEQVPoint(struct SEquivalentModifyMsg *pMsg);
    BOOL NotifyEncouragement(struct SAEncouragement *pMsg);
	void QuestEncouragement(char *szName,char * szID);

    BOOL DispatchGMIDCheckMsg(DNID dnidClient, struct SQCheckGMInfMsg *pMsg, CPlayer *pPlayer);                 // ����GM�����֤
    inline  DWORD   GetGameTick(void)  { return  m_dwGameTick; }
  
    // ˢ�����а�
    BOOL GWSaveScores();
    BOOL GWLoadScores();
    void UpdateNewXYDScore( SFixProperty *p );
	///@brief ��ȡ��ҹϵͳ��ģ���߼�
	BOOL GetLuckySystemPtr(CLuckyTimeSystem** ptr);

	// XYD3���а�~�����
	void UpdateXYD3RankList(CPlayer *player);
	BOOL LookRankListEquipInfo(CPlayer *pSrcPlayer, CPlayer *pDestPlayer);
public:
    void DisplayServerInfo(char i);
	void syneffects(DWORD dwgid, WORD weffectsindex, WORD weffectsPosX, WORD weffectsPosY);//����һЩ��Ч��ͬ��
    BOOL UpdatePlayerData(struct SFixBaseData * pPlayerBaseData);
//-----------------------------------------------------------------
// ��Ϣ�������б���
public:
	BOOL OnLogout(DNID dnidClient, struct SQLogoutMsg *pMsg, CPlayer *pPlayer);

public:
    BOOL RecvPrepareExchangeResult(struct SAPrepareExchangeMsg *pMsg);

	//void UpdateBossData(WORD id,BossDeadData &monsterdata);
	void UpdateBossData(BossDeadData &monsterdata);
	//void RecvUpdateBossData(SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer);
	void RecvUpdateBossData(SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer);
	void RecvActivityNotice(SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer);
	void AddToBuff(BYTE **pBuff,WORD datavalue);
	
	void SetActivityNotice(ActivityNotice &aNotice);
	void LoadActivityNotice();
	void RecvGetActivityNotice(SQBossDeadUpdatemsg *pmsg, CPlayer *pPlayer);
	bool LoadBlessOpenClose();
	void SendBlessOpenClose();
	BOOL SendMsgToLoginSrv(struct SMessage *pMsg, int iSize);
//-----------------------------------------------------------------
// ���̿��ƺ���
public:
	BOOL Initialize();
	void Run();
	void Destroy();
	void OnReadBlessFile();
	void OnWriteBlessFile();
	void OnWriteArenaRankFile();
public:
	CGameWorld(void);
	~CGameWorld(void);
	void AddGlobalBlessInfo(std::string name,std::string ItemName);
	void ProcessBlessInfo(DNID Nid,BYTE flag);
//-----------------------------------------------------------------
// ������
private:
    class DUpgradeSys *m_pUpgradeData;      // ���ɻ�������

    // ��Ϸ�����࣬ÿ1000����+1,��̫��ȷ
    DWORD       m_dwGameTick;
	DWORD		m_dgwordTick;
	GLOBALBLESSINFO m_GlobalInfo;
	std::shared_ptr<CLuckyTimeSystem> m_ptrLuckySystem;
public:

    TeamManager m_TeamManager;

    CFactionManager m_FactionManager;
    UnionDataManager m_UnionManager;

	BossDeadDataTable m_BossDeadData;
	DWORD		m_BossupdateTime;
	BYTE			m_BossState[128];
	ActivityNoticeTable m_activityTable;
	DWORD m_activityUpdateTime;
	BYTE  m_BlessOpenClose;
#ifdef GMMODULEON
    CNetGMModule m_NetGMModule;
#endif
};

struct SGMCheck
{
    SGMCheck(){};
    WORD wGameType;            // ��Ϸ���1��ʾ��1��2��ʾ��2��3��ʾ������4��ʾ����꣬5��ʾʥ��ʿ
    DWORD dwServerid;          // serverid ������ID
    DNID mac;                  // mac GM Mac��ַ
    DWORD dwRand;              // rnd �����
    WORD wGMCmd;               // cmd GM����
    DWORD sid;                 // ���SID
    string cradKey;            // Ч��KEY  
    string ip;                 // IP
    string account;            // account
    SQGameMngMsg msg;          // ����GM����Ϣ
};

extern CGameWorld *&GetGW();
