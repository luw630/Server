#pragma once
#include "stdint.h"
#include <memory>
#include <functional>
#include"HeroMoudle\HeroDataManager.h"
//#include "player.h"

class CPlayer;
class CDManager;
struct SFixData;
class CStorageManager;
class SSGPlayerMsg;
class CAchieveUpdate;
class CRandomAchieveUpdate;
struct STimeData;
class CEnduranceManager;
class CMissionUpdate;
class CGuideManager;
enum FunctionMoudleType;

//@brief ���������������ͨ�õ�����
class CBaseDataManager : public CHeroDataManager
{
public:
	CBaseDataManager(DNID clientIndex, SFixData* pPlayerData, CPlayer * _cplayer);
	~CBaseDataManager();

	void Init(bool bGMFlag);
	/**@brief ��������ݷ����ͻ���֮ǰ������һЩ���ݵĳ�ʼ����������Ҫ���ڸ�lua�����Ĳ��ֵ����ݵĳ�ʼ��*/
	void InitDataOfLogin();
	///@brief ���óɾ͵ĸ����߼�ģ��
	void SetAchievementUpdator(CAchieveUpdate* pAchievement);
	///@brief ��������ɾ͵ĸ����߼�ģ��
	void SetRandomAchievementUpdator(CRandomAchieveUpdate* updator);
	///@brief ��������ĸ����߼�ģ��
	void SetMissionUpdator(CMissionUpdate* updator);
	int32_t GetMasterLevel();
	///@���ص����ĸ���
	DWORD GetGoods(int32_t iType, int32_t id = 0);
	///@return ����Ӧ�÷��͸��ͻ��˵ġ��������������佫����Ʒ�������¼ӵĸ���������������Ǯ����ʯ�����Ǽ���ָ������֮����ܸ���
	DWORD AddGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0, int32_t detailType = 0, bool IsSynToClient = true);
	///@brief �������ĳһ���Goods����ͳһ����Ϣ���ͻ���
	///@param iType Ҫ��ӵ�Goods������
	///@param IDList Ҫ��ӵ���Ʒ��ID�б�Ϊһ����̬����
	///@param IDNum Ҫ��ӵ���Ʒ��ID�ĸ���
	///@param numList Ҫ��ӵĲ�ͬ����Ʒ��Ӧ�ĸ���
	///@note ��ע��IDList ��numList�ĸ���һ��ҪΪIDNum(ǰ����IDList��Ϊ��)
	bool AddGoods_SG(int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] = nullptr, int32_t detailType = 0);
	///@return ����Ӧ�÷��͸��ͻ��˵ġ��������������佫����Ʒ������Ҫ��ȥ�ĸ���������������Ǯ����ʯ�����Ǽ���ָ������֮����ܸ���
	DWORD DecGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0, int32_t detailType = 0, bool IsSynToClient = true);
	///@brief ����ɾ��ĳһ���Goods����ͳһ����Ϣ���ͻ���
	///@param iType Ҫɾ����Goods������
	///@param IDList Ҫɾ������Ʒ��ID�б�Ϊһ����̬����
	///@param IDNum Ҫɾ������Ʒ��ID�ĸ���
	///@param numList Ҫɾ���Ĳ�ͬ����Ʒ��Ӧ�ĸ���
	///@note ��ע��IDList ��numList�ĸ���һ��ҪΪIDNum(ǰ����IDList��Ϊ��)
	bool DecGoods_SG(const int32_t iType, int IDNum, int32_t numList[], const int32_t IDList[] = nullptr, int32_t detailType = 0);
	bool CheckGoods_SG(const int32_t iType, const int32_t id = 0, int32_t num = 0);

	int32_t GetMoney() const;
	int32_t GetEndurance() const;
	int32_t GetDiamond() const;
	uint32_t GetMaxRecharge() const;
	int32_t GetHonor() const;
	int32_t GetExploit() const;
	int32_t GetPrestige() const;
	DNID GetDNID(){return m_ClientIndex; }
	void SetDNID(DNID newID) { m_ClientIndex = newID; }
	DWORD GetSID() const;
	std::string GetName();
	
	//�������Ψһ��
	int CheckGameName(char* newname,DWORD SID);

	//����ڴ�����
	int SetGameName(char* newname);

	//int GetChangeNameCount();
	bool Release();

	///@brief ��Ǯ���ϴ������
	DWORD PlusMoney(const int32_t value);

	///@brief ���Ӿ���
	int PlusExploit(int32_t value);

	///@brief ��ʯ���ϴ������
	DWORD PlusDiamond(const int32_t value);

	///@�����г�ֵ����
	DWORD AddMaxRechage(const uint32_t value);

	///@brief ��������
	int PlusHonor(int32_t value);

	///@brief ��������
	int PlusPrestige(int32_t value);

	///@brief �޸����������������
	int ModifyBlessPoints(int value);

	///@brief �޸Ľ���ֽ������Ļ��
	int ModifySoulPoints(int value);

	///@brief �޸ľ��Ż�����ľ���
	int ModifyToken(int value);

	///@brief �������Ӿ��������Ҳ����ͻ��˷���Ϣ,��֧�ּӸ���
	void PlusExp(int32_t value);

	///@brief �������Ʒ
	DWORD AddItem(const int32_t item, const int32_t itemCount = 1);

	///@brief �Ƿ������˺�
	bool IsNewPlayer();
	
	///@brief ��������������µ�Ӣ�ۣ���ʱֻ�����Ķ�CHeroDataManager��һ���װ��������¸���Ӣ���ռ��ɾ͵��߼�
	///@TODO �ع�@Joson
	virtual bool AddHero(const int32_t heroID);
	///@brief ���������ݱ��˵�ʱ��
	virtual void OnArenaDefenseTeamChanged();

	///@breif װ���佫
	bool EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType, const int32_t equipLevelLimit);
	///@brief ����ָ��Ӣ�۵ľ���ֵ����֧������ �����ľ���ֵ
	///@param heroID Ҫ������Ӣ�۵�ID
	///@param value[int/out] Ҫ���ӵľ���ֵ������,���ӳɹ��󷵻����Ӻ��ֵ
	///@param level[int/out] �Ż����Ӿ����ĵȼ�
	///@param levelLimitted[out] ����佫�ȼ��Ƿ�嶥��
	bool PlusHeroExp(int heroID, OUT int& value, OUT int& level, OUT bool& levelLimitted);

	///@breif �����佫����
	//virtual bool UpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t skillLevel);

	///@brief ��ʱ��ʱ�ڴ˴����佫�����߼������ڴ������׹������ж��Ƿ���װ����������ж�����ͷ���һ����������ʯ
	///@NOTE �ع�@Jonson
	virtual bool HeroRankRise(const int32_t heroID, const DWORD curRank);
	virtual bool HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel);

	///@brief�µ�һ�������������
	void ResetDataForNewDay();

	///@brief��ȡCD������
	CDManager &GetCDManager() { return (*m_upCdMgr); }

	///@breif ��ȡ��һ�μ�������ʱ��
	//DWORD GetLastUpgradeHeroSkillTime();
	///@breif ������һ�μ�������ʱ��
	//void SetLastUpgradeHeroSkillTime(const DWORD timePar);

	///@breif ��ȡ���ܵ�
	//DWORD GetSkillPoint();
	///@breif ���ü��ܵ�
	//void SetSkillPoint(const DWORD skillPointNum);

	///@brief����CD������
	//void SetCDManager(CDManager * pCDMgr){ m_pCdMgr = pCDMgr; }
	CStorageManager& GetStorageManager() { return *m_upStorageMgr; }
	///@brief ��ʱ��ª��������������ģ��������ɾ���ص��߼����µ�ģ�鶼�������ȡ�ɾ͸��µ����
	///@NOTE: �ع�@Jonson
	CAchieveUpdate* GetLifeTimeAchievementUpdator();
	///@brief ��ʱ��ª��������������ģ�������ɾ���ص��߼����µ�ģ�鶼�������ȡ�ɾ͸��µ����
	///@NOTE: �ع�@Jonson
	CRandomAchieveUpdate* GetRandomAchievementUpdator();
	///@brief ��ʱ��ª��������������ģ���������ص��߼����µ�ģ�鶼�������ȡ������µ����
	///@NOTE: �ع�@Jonson
	CMissionUpdate* GetMissionUpdator();
	///@brief �������߼���ʱ�ŵ�CBaseDataManager�У�����Ҫ��Ҫ������ȥ@��ΪΪ
	void OnRecvStorageOperation(SSGPlayerMsg* pMsg);
	/*@brief ��������������Ϣ*/
	void OnRecvGuideOperation(SSGPlayerMsg* pMsg);
	/*@ָ��������������ж�ǰ����һ�� ����Ҫ������Ϣ ���ڴ˴���*/
	void ProcessOperationOfGuide(FunctionMoudleType functionType, int rewardID = 0);
	
	const int64_t GetLogoutTime() { return m_LogoutTime; }
	const int64_t GetLoginTime() { return m_LoginTime; }
	DWORD GetPlayerLevel() const;
	const STimeData& GetTimeData() const;
	//��ʱ��GM��ǣ����ڲ����Ժ���Ҫ�޸�
	bool GetGMFlag() const { return m_bTempGMFlag; }
	void SetGMFlag(bool flag){ m_bTempGMFlag = flag; }
	// ��ȡ�����������������
	CGuideManager& GetGuideManager() { return *m_upGuideMgr; }
	/// �ж��Ƿ����佫�ɽ��� 
	void JudgeHeroUpgradeRank(int itemID);
	/// �ж��Ƿ���Լ���Ե������
	virtual bool CanCalculateFateAttribute();
	
	///@brief ��ȡiconID
	DWORD GetIconID();
	///@brief VIP�ȼ�����ʽ�����������
	void UpdateDateForVIP(int vipLevel);
protected:
	///������ս����֮���������
	virtual void ProcessAfterCombatPowerCalculated(int heroID, int combatPower);

private:
	bool _checkSFixData();

	SFixData* m_pPlayerData;
	int64_t m_LoginTime;
	int64_t m_LogoutTime;
	DNID m_ClientIndex;
	int32_t m_CurItemNum;
	CAchieveUpdate* m_ptrAcievementUpdate; ///<�����佫���ס��佫�ռ�����ء��������ɾ��߼� TODO:��ʱ����������ʳɾ͵��߼���,��������Ҫ��Ҫ�ع�
	CRandomAchieveUpdate* m_ptrRandomAchievementUpdate;///<�����佫���ס��佫�ռ��������ɵĳɾ͵��߼�
	CMissionUpdate* m_ptrMissionUpdate;	///<����Ľ��ȸ����߼�
	std::unique_ptr<CDManager> m_upCdMgr;
	std::unique_ptr<CStorageManager> m_upStorageMgr;
	std::unique_ptr<CEnduranceManager> m_upEnduranceMgr;
	std::unique_ptr<CGuideManager> m_upGuideMgr;
	bool m_bTempGMFlag;
	std::function<void(int)> m_funcJudgeUpgradeRank;
	CPlayer*  cPlayer;
};

