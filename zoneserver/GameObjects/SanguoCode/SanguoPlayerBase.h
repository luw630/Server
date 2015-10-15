#pragma once
#include <memory>
#include "..\CDManager.h"
#include "Networkmodule\SanguoPlayerMsg.h"
#include <unordered_map>
#include "ItemUser.h"
const __int32 IID_SANGUOPLAYER = 0x117ca77a;

struct SFixData;
struct SBlessData;
struct SHeroMsg;
class CBaseDataManager;
class CExtendedDataManager;
class CMallManager;
class CDessertSystem;
class CDuplicateBaseManager;
class CMissionManager;
class CDiamondExchangeProcess;
class CExpeditionDataManager;
class CDuplicateDataManager;
class CDuplicateModule;
class CMissionDataManager;
class CDiamondExchangeModule;
class CAchievementDataManager;
class CSkillUpgradeManager;
class CRandomAchieveDataManager;
class CRandomAchieveManager;
class CChaseThiefDataManager;
class CChaseThiefTollgateManager;
class CHeroExtendsGameplayModel;
enum MallType;
enum InstanceType;

class CArenaDataMgr;
class CAchievementManager;
enum DataMgrType
{
	None = -1,
	Bless_Mgr = 0,
	ExpeditionMgr,
	MissionMgr,
	Arena_Mgr,
	Max 
};

class CSanguoPlayer: public CItemUser
{
public:
	CSanguoPlayer();
	~CSanguoPlayer();

	int OnCreate(_W64 long pParameter);
	//void OnRecv();
	//改名方法
	void ChangeGameName(SQChangePlayerName * pMsg);
	void Db_Back_ChangeGameName(char * name, DWORD SID, CBaseDataManager* baseDataMgr);

	void OnRecvSanguoPlayerRequest(SSGPlayerMsg * pMsg);
	void SetSanguoPlayerData(SFixData* pFixData);
	void SetCplayerPtr(CPlayer*  _cPlayer);
	void SetSanguoPlayerDNID(const DNID dnid) { m_ClientIndex = dnid; }
	bool Release();
	void SetGMFlag(const bool flag){ m_GMFlag = flag; }
	void OnRecvStorageOperation(SSGPlayerMsg* pMsg);
	CBaseDataManager * GetSanguoBaseData();
	void OnRun();
	CBaseDataManager* GetBaseDataMgr();
	CExtendedDataManager* GetDataMgr(DataMgrType type);
	const CDuplicateBaseManager* GetDuplicatemanager(InstanceType type);
	CDessertSystem* GetDessertSystem();
	bool SanguoLogout();
	/**@brief VIP等级提升时更新相应数据*/
	void UpdateDataForVIP(int vipLevel);
	/**@brief 在玩家数据发给客户端之前，进行一些数据的初始化，现在主要用于跟lua交互的部分的数据的初始化*/
	void InitDataOfLogin();
	void FristLogin(); //在区域服数据首次登录（不是指新号登录）
public:
	///@breif 凌晨五点的刷新点的事件
	void FiveOclockRefreshEvent();
	///@breif 中午12点的刷新点的事件
	void TwelveOclockRefreshEvent();
	///@breif 下午14点的刷新点的事件
	void FourteenOclockRefreshEvent();
	///@breif 下午18点的刷新点的事件
	void EighteenOclockRefreshEvent();
	///@breif 晚上20点的刷新点的事件
	void TwentyOclockRefreshEvent();
	///@breif 晚上21点的刷新点的事件
	void TwentyOneOclockRefreshEvent();
	///@breif 晚上24点的刷新点的事件
	void TwentyFourOclockRefreshEvent();
	///@brief 杂货铺商品刷新事件
	void VarietyShopRefreshEvent();
	///@brief 竞技场商店商品刷新事件
	void ArenaShopRefreshEvent();
	///@brief 远征商店商品刷新事件
	void ExpeditionShopRefreshEvent();
	///@brief 军团商店商品刷新事件 
	void LegionShopRefreshEvent();
	///@brief 奇缘商人商品刷新事件
	void MiracleMerchantRefreshEvent();
	///@brief 珍宝商人商品刷新事件
	void GemMerchantRefreshEvent();
	///@brief 将魂商店商品刷新事件
	void SoulPointExchangeShopRefreshEvent();
	///@brief 国战商店商品刷新事件
	void WarOfLeagueShopRefreshEvent();
	///@brief 系统到达了黑夜时间
	void LuckyTimeComing();

public:
	///@brief 暂时供lua逻辑调用的加技能点的接口，重构@jonson
	void AddSkillPoint(int num);
private:
	//CDManager m_CDmgr;


	void InitGMData();
	DNID m_ClientIndex;
	SFixData*  m_pPlayerData; //存储所有的数据
	std::unique_ptr<CBaseDataManager> m_upBaseDataMgr;
	std::unique_ptr<CExpeditionDataManager> m_ExpeditionDataMgr;
	std::unique_ptr<CMissionManager> m_MissionMgr;
	std::unique_ptr<CMissionDataManager> m_MissionDataMgr;
	std::unique_ptr<CAchievementDataManager> m_AchievementDataMgr;
	std::unique_ptr<CDuplicateDataManager> m_DuplicateDataMgr;
	std::unique_ptr<CAchievementManager> m_AchievementMgr;
	std::unique_ptr<CRandomAchieveDataManager> m_RandomAchievementDataMgr;
	std::unique_ptr<CRandomAchieveManager> m_RandomAchievementMgr;
	//std::unique_ptr<CArenaDataMgr> m_ArenaDataMgr; //竞技场数据管理
	std::unique_ptr<CSkillUpgradeManager> m_SkillUpgradeMgr;
	std::unique_ptr<CDiamondExchangeModule> m_DiamondExchangeModule;
	std::unique_ptr<CDessertSystem> m_DessertSystem;
	std::unique_ptr<CChaseThiefDataManager> m_ChaseThiefDataMgr;
	std::unique_ptr<CChaseThiefTollgateManager> m_ChaseThiefTollgateMgr;
	std::unique_ptr<CHeroExtendsGameplayModel> m_HeroExtendsGameplayMgr;
	std::vector<std::unique_ptr<CExtendedDataManager>> m_DataMgrList;
	std::map<MallType, std::unique_ptr<CMallManager>> m_MallMgrList;
	std::unordered_map<InstanceType, shared_ptr<CDuplicateBaseManager>> m_duplicateMgrs;
	bool m_GMFlag;
	CPlayer*  cPlayer;
};
enum TipMessage
{
	
};