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
	//��������
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
	/**@brief VIP�ȼ�����ʱ������Ӧ����*/
	void UpdateDataForVIP(int vipLevel);
	/**@brief ��������ݷ����ͻ���֮ǰ������һЩ���ݵĳ�ʼ����������Ҫ���ڸ�lua�����Ĳ��ֵ����ݵĳ�ʼ��*/
	void InitDataOfLogin();
	void FristLogin(); //������������״ε�¼������ָ�ºŵ�¼��
public:
	///@breif �賿����ˢ�µ���¼�
	void FiveOclockRefreshEvent();
	///@breif ����12���ˢ�µ���¼�
	void TwelveOclockRefreshEvent();
	///@breif ����14���ˢ�µ���¼�
	void FourteenOclockRefreshEvent();
	///@breif ����18���ˢ�µ���¼�
	void EighteenOclockRefreshEvent();
	///@breif ����20���ˢ�µ���¼�
	void TwentyOclockRefreshEvent();
	///@breif ����21���ˢ�µ���¼�
	void TwentyOneOclockRefreshEvent();
	///@breif ����24���ˢ�µ���¼�
	void TwentyFourOclockRefreshEvent();
	///@brief �ӻ�����Ʒˢ���¼�
	void VarietyShopRefreshEvent();
	///@brief �������̵���Ʒˢ���¼�
	void ArenaShopRefreshEvent();
	///@brief Զ���̵���Ʒˢ���¼�
	void ExpeditionShopRefreshEvent();
	///@brief �����̵���Ʒˢ���¼� 
	void LegionShopRefreshEvent();
	///@brief ��Ե������Ʒˢ���¼�
	void MiracleMerchantRefreshEvent();
	///@brief �䱦������Ʒˢ���¼�
	void GemMerchantRefreshEvent();
	///@brief �����̵���Ʒˢ���¼�
	void SoulPointExchangeShopRefreshEvent();
	///@brief ��ս�̵���Ʒˢ���¼�
	void WarOfLeagueShopRefreshEvent();
	///@brief ϵͳ�����˺�ҹʱ��
	void LuckyTimeComing();

public:
	///@brief ��ʱ��lua�߼����õļӼ��ܵ�Ľӿڣ��ع�@jonson
	void AddSkillPoint(int num);
private:
	//CDManager m_CDmgr;


	void InitGMData();
	DNID m_ClientIndex;
	SFixData*  m_pPlayerData; //�洢���е�����
	std::unique_ptr<CBaseDataManager> m_upBaseDataMgr;
	std::unique_ptr<CExpeditionDataManager> m_ExpeditionDataMgr;
	std::unique_ptr<CMissionManager> m_MissionMgr;
	std::unique_ptr<CMissionDataManager> m_MissionDataMgr;
	std::unique_ptr<CAchievementDataManager> m_AchievementDataMgr;
	std::unique_ptr<CDuplicateDataManager> m_DuplicateDataMgr;
	std::unique_ptr<CAchievementManager> m_AchievementMgr;
	std::unique_ptr<CRandomAchieveDataManager> m_RandomAchievementDataMgr;
	std::unique_ptr<CRandomAchieveManager> m_RandomAchievementMgr;
	//std::unique_ptr<CArenaDataMgr> m_ArenaDataMgr; //���������ݹ���
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