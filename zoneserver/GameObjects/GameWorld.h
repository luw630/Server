#pragma once
#include <memory>
#include "playermanager.h"
#include "regionmanager.h"
#include "dchatmanager.h"
#include "teammanager.h"
#include "unionmanage.h"
#include "factionmanager.h"
#include "区域服务器/区域服务器/NetGMModule.h"
#include "networkmodule/chatMsgs.h"
#include "StackWalker.h"
#include "SceneMap.h"
#include "Transformers.h"
#include "ArenaMap.h"
class   CPlayer;
class CLuckyTimeSystem;
struct SQSynSimPlayerMsg;

//typedef std::hash_map<WORD, BossDeadData>BossDeadDataTable;		//<怪物ID，表>
typedef std::vector<BossDeadData>BossDeadDataTable;		//<怪物ID，表>
typedef std::vector<ActivityNotice>ActivityNoticeTable;		//<日常活动>
struct GLoblessInfoSTR
{
	std::string name;
	std::string ItemName;
};
class CGameWorld : 
	public CRegionManager, public CPlayerManager, public CDChatManager
{
//-----------------------------------------------------------------
// 私有函数
typedef std::list<GLoblessInfoSTR> GLOBALBLESSINFO;
public:
    int GetAccountLimit( CPlayer *pPlayer );                             // 获取账号权限
//-----------------------------------------------------------------
// 一些接口函数
public:
	BOOL OnDispatch(DNID dnidClient, struct SMessage *pMsg, CPlayer *pPlayer);

    // 服务期间数据传送相关的消息处理
    BOOL OnDispathDTM(SMessage *pMsg, WORD wMsgSize, CPlayer *pPlayer);
    BOOL OnDispathDTM(SMessage *pMsg, WORD wMsgSize);

    // 客户端数据传送相关的消息处理
	BOOL DispatchSysMessage(DNID dnidClient, struct SSysBaseMsg *pMsg, CPlayer *pPlayer); // 具体的消息派发函数
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
    BOOL DispatchSceneMessage(DNID dnidClient, struct SSceneMsg *pMsg, CPlayer *pPlayer); //关卡场景消息
	BOOL DispatchTransformersMessage(DNID dnidClient, struct STransformersMsgs *pMsg, CPlayer *pPlayer); //变身消息
	BOOL DispatchDyArenaMessage(DNID dnidClient, struct SArenaMsg *pMsg, CPlayer *pPlayer); //竞技场消息
	BOOL DispatchDyArenaMessage(DNID dnidClient, SSGArenaMsg *pMsg, CPlayer *pPlayer); //三国竞技场
#ifdef GMMODULEON
    BOOL DispatchGMModuleMessage(DNID dnidClient,struct SGMMMsg * pMsg,CPlayer *pPlayer,BOOL bDistribute);
#endif

//-----------------------------------------------------------------
// 一些具体功能函数
    
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

	// 元宝积分的刷新
	BOOL RefreshPlayerMoneyPoint(struct SARefreshMoneyPointMsg *pMsg);

	// 侠义币换点卡
	BOOL NotifyMCInfo(struct SExchangeMCMsg *pMsg);

	// EQVPoint通知
	BOOL NotifyEQVPoint(struct SEquivalentModifyMsg *pMsg);
    BOOL NotifyEncouragement(struct SAEncouragement *pMsg);
	void QuestEncouragement(char *szName,char * szID);

    BOOL DispatchGMIDCheckMsg(DNID dnidClient, struct SQCheckGMInfMsg *pMsg, CPlayer *pPlayer);                 // 处理GM身份验证
    inline  DWORD   GetGameTick(void)  { return  m_dwGameTick; }
  
    // 刷新排行榜
    BOOL GWSaveScores();
    BOOL GWLoadScores();
    void UpdateNewXYDScore( SFixProperty *p );
	///@brief 获取黑夜系统的模块逻辑
	BOOL GetLuckySystemPtr(CLuckyTimeSystem** ptr);

	// XYD3排行榜~新添加
	void UpdateXYD3RankList(CPlayer *player);
	BOOL LookRankListEquipInfo(CPlayer *pSrcPlayer, CPlayer *pDestPlayer);
public:
    void DisplayServerInfo(char i);
	void syneffects(DWORD dwgid, WORD weffectsindex, WORD weffectsPosX, WORD weffectsPosY);//处理一些特效的同步
    BOOL UpdatePlayerData(struct SFixBaseData * pPlayerBaseData);
//-----------------------------------------------------------------
// 消息处理函数列表区
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
// 流程控制函数
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
// 数据区
private:
    class DUpgradeSys *m_pUpgradeData;      // 门派基本数据

    // 游戏主节奏，每1000毫秒+1,不太精确
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
    WORD wGameType;            // 游戏类别1表示侠1，2表示侠2，3表示新侠，4表示侠棋魂，5表示圣斗士
    DWORD dwServerid;          // serverid 服务器ID
    DNID mac;                  // mac GM Mac地址
    DWORD dwRand;              // rnd 随机数
    WORD wGMCmd;               // cmd GM命令
    DWORD sid;                 // 玩家SID
    string cradKey;            // 效验KEY  
    string ip;                 // IP
    string account;            // account
    SQGameMngMsg msg;          // 本次GM的消息
};

extern CGameWorld *&GetGW();
