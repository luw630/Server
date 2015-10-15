// ----- SkillUpgradeManager.h ------
//	 --  Author : LiuWeiWei
//	 --  Date : 3/4/15 ��עʱ��
//   --  Desc : �����佫���ܵ����� ���ܵ�Ĺ���
#pragma once
#include "../ExtendedDataManager.h"
struct SSkillPointData;
class CBaseDataManager;
class CMissionUpdate;
struct CGlobalConfig;

class CSkillUpgradeManager
	: public CExtendedDataManager
{
public:
	CSkillUpgradeManager(CBaseDataManager& BaseDataMgr);
	virtual ~CSkillUpgradeManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	/**@brief ��������ϴ����߻��״ε�½���ظ��ļ��ܵ�*/
	bool InitSkillPointData();
	/**<@brief ��������
	@note ���ܵȼ����ڻ�����佫�ȼ�
	@param [in] heroID �佫ID
	@param [in] skillID ����ID
	@param [in] skillLevel ���ܵȼ�
	*/
	bool UpgradeSkill(const int heroID, const int skillID, const int skillLevel);
	bool MultiUpgradeSkill(const int heroID, const int skillID, const int destLevel, DWORD& skillPointCost);
	/**@brief ���ݼ���ID��Ŀ��ȼ������������ɸ�����
	@param [in] heroID Ӣ��ID
	@param [in] skillIDs ����ID����
	@param [in] destLevel Ŀ��ȼ�����
	@param [in] ������ЧԪ�ظ���
	@param [out] skillPointCost ���ļ��ܵ���
	*/
	bool MultiUpgradeSkill(const int heroID, DWORD skillIDs[], DWORD destLevel[], DWORD validNum, DWORD& skillPointCost);
	///@breif �Լ��ܵ�ҩ���ȼӼ��ܵ�
	///@param Num ��ӵļ��ܵ���
	///NOTE:�˽ӿڽ������е�ʹ�ü��ܵ�ҩ����lua�߼����ã���Ϊ������������������Ϣ�����ߵ���ͨ�õ�ʹ������Ʒ���߼����������ڿͻ��������Լ��ؼ��ܵ���߼�,
	///�ĳ���AddGoods��һ��Ҫ�ѵ�ʱ�䣬��ʱ�ṩ��ôһ���ӿڱ�¶��ȥ���ع�@jonson
	void AddSkillPoint(int Num);
	///<@brief �����ܵ�
	bool BuySkillPoint();
	///<@brief �ɷ�ѧϰ����
	bool CanLearnSkill();
	///<@brief �ѹ�������ܵ�Ĵ���
	DWORD PurchasedSkillPointTimes() const;
	///<@brief ʣ��ļ��ܵ���
	DWORD RemainingSkillPoint() const;
	///<@brief ʣ��ظ�ʱ��
	DWORD RecoverRemainingTime() const;
	///<@brief ������ѧϰ���ܱ�־(���ڿͻ��˵ļ��������ж�)
	void SetLearnedSkill();
	/**@brief VIP�ȼ�����ʱ�ı似�ܵ����� 
	@param [in] vipLevel ��ǰVIP�ȼ�
	*/
	bool SetSkillPointForVIP(int vipLevel);
private:
	void _CalculateSkillPoint(const __time64_t& lastTime, const __time64_t& currentTime);
private:
	SSkillPointData* m_pSkillPointData;
	CGlobalConfig& m_GlobalConfig;
	__time64_t m_LastCalcSkillPointTime;
	__time64_t m_CurrentTime;
	int m_iSkillPointLimit;	///<���ܵ�����(VIP��ҿ�����)//
	CMissionUpdate* m_ptrMissionUpdator;
};

