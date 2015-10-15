#pragma once
#include"..\Common\ConfigManager.h"
#include <memory>
#include <functional>

using namespace std;
enum HeroType;
struct SHeroData;
struct SHeroEquipment;

class CHeroDataManager
{
public:
	CHeroDataManager(SHeroData *pHeroArray);
	~CHeroDataManager();
	bool Init();
	bool Release();
	virtual bool AddHero(const int32_t heroID);
	///@brief ���������ݱ��˵�ʱ��
	virtual void OnArenaDefenseTeamChanged();

	bool ExistHero(const int32_t heorID) const;
	///@brief ���þ�������������ID
	void SetArenaDefenseTeam(DWORD arenaDefenseTeamID[5]);
	///@brief ��ȡ��������������ID
	const DWORD* GetArenaDefenseTeam() const { return m_pDefenseTeamID; }
	///@brief ��������佫��Զ�������е�Ѫ����ħ��ֵ����Ϣ
	void ResetExpedtiionInfor();
	///@brief ��������佫��Զ�������е�һЩѪ����ֵ��ħ����ֵ
	void SetExpeditionInfor(int32_t heroID, float healthScale, float manaScale);
	const SHeroData* GetExcellentHeroInfor();
	const SHeroData* GetHero(const int32_t heorID) const;
	///@brief ���Ӣ�۵�����
	int GetHeroNum() const;
	///@brief ��ȡ����Ӣ�۵��Ǽ���Ӧ���ܺ�
	int GetHeroStarSum() const;
	int GetHeroLevel(int heroID) const;
	///@brief ��ȡӢ��ĳһ��װ������Ϣ
	const SHeroEquipment* GetHeroEquipment(int32_t heroID, int32_t equipID)const;
	///@brief ��ȡĳһӢ�۵�����װ����Ϣ
	const SHeroEquipment * const GetHeroEquipments(int32_t heroID)const;
	///@brief ĳһ��װ���Ƿ�ָ����Ӣ����װ��
	bool IsHeroEquipmentAttached(int32_t heroID, int32_t equipID) const;
	///@brief ����ĳһ��Ӣ�۵�ĳһ��װ���Ķ��쾭��
	///@param starLevel ���������,�����������Ǽ����س��������û��������������֣��򷵻�0
	///@return �ɹ��������ӵľ���ֵ��ʧ�ܷ���-1
	int IncreaseHeroEquipmentForgingExp(int32_t heroID, int32_t equipID, int32_t exp, OUT int32_t& starLevel);
	
	///@brief ��ȡ���������������佫��Ϣ
	void GetArenaDefenseTeamInfo(SHeroData* pHeroDataList);
	
	///@brief ��ȡ��ǿ��5���佫
	void GetTop5Hero(SHeroData* pHeroDataList);

	virtual bool EquipHero(const int32_t heroID, const int32_t euquipID, const int32_t equipType);
	///@breif ���佫��װ������֮��Ĳ�����������ʱ��Ҫ���ڵ�һ���Դ����װ����ʱ�򣬷���������Ӣ�����Ե��������Ż�
	void EquipHeroEndProcess(int32_t heroID);
	virtual bool HeroStarLevelRise(const int32_t heroID, const DWORD curStarLevel);
	void IncreaseHeroRank(int32_t heroID);
	bool CanRiseRank(const int32_t heroID) const;
	bool CanRiseStarLevel(const int32_t heroID) const;
	virtual bool UpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t skillLevel);
	bool MultiUpgradeHeroSkill(const int32_t heroID, const int32_t skillID, int32_t destLevel);
	bool MultiUpgradeHeroSkill(const int32_t heroID, DWORD skillIDs[], DWORD destLevels[], DWORD validNum);
	///@brief ��ȡӢ�ۼ��ܵȼ�
	DWORD GetHeroSkillLevel(const int32_t heroID, const int32_t skillID) const;
	///@brief �������Ӣ��ID
	void GetAllHeroID(vector<int32_t>& vecID);
	///@brief ��ȡս����
	int GetCombatPower();
	///@brief ��ȡ�佫ȥ��Ե�����Ժ������
	bool GetHeroAttributeWithoutFateAttribute(int32_t heroID, OUT CharacterAttrData& attr) const;
	///@brief ������ָ���佫Ե����ص��佫����
	///@param [in] heroID ָ���佫ID
	void UpdateRelatedHeroesAttribute(int32_t heroID);
	///@brief �ܷ����Ե������
	virtual bool CanCalculateFateAttribute();
	///@brief ���������佫��Ե������
	void UpdateFateAttributeOfAllHeroes();
	///@brief ��ȡ�佫����
	///param [in] heroID ָ���佫ID
	const SHeroAttr* GetHeroAttr(int32_t heroID) const;
	///@brief ��ȡ�佫�����Ե����������ֵ
	///@param [in] heroID ָ����ȡ���佫ID
	///@param [out] attrArray ��������������
	///@param [in] arraySize �����������С
	bool GetHeroFateAttribute(int32_t heroID, int attrArray[], const int arraySize) const;
protected:
	struct ExcellentHeroInfo
	{
		int32_t HeroID;
		int32_t CombatPower;
	};

	///������ս����֮���������
	virtual void ProcessAfterCombatPowerCalculated(int heroID, int combatPower);
	///@brief ɾ��ָ��Ӣ�۵�����װ����Ϣ
	void ClearHeroEquipment(int heroID);
	///@brief ����ָ��Ӣ�۵ľ���ֵ����֧������ �����ľ���ֵ
	///@param heroID Ҫ������Ӣ�۵�ID
	///@param value[in] Ҫ���ӵľ���ֵ������
	///@param level[in] �����ĵȼ�
	bool PlusHeroExp(int heroID, int value, int level);

	bool m_bDataInitialed;
	int32_t m_HeroNum;
	SHeroData *m_pHeroArray;
	DWORD *m_pDefenseTeamID;
	map<int32_t, int32_t, greater<int32_t>> m_ExcellentHeroInfor;///<keyֵΪս������valueֵΪӢ��ID,��������,���������λӢ�۵�ս����
	unordered_map<int32_t, SHeroData* > m_heroList;
	unordered_map<int32_t, std::shared_ptr<SHeroAttr>> m_heroAttrlist;///Ӣ�۵�����ֵ����

private:
	SHeroData * _GetHero(const int32_t hero);
};


