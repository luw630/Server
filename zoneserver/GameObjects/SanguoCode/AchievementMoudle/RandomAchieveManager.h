// ----- CRandomAchieveManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   �ȶ�������������ɵĿɴ�ɵĳɾ͵��������߼��Ĺ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
#include "RandomAchieveUpdate.h"

class CLuckyTimeSystem;
class CExtendedDataManager;
class CRandomAchieveDataManager;
struct SRandomAchievementMsg;
struct SAchivementUnitData;
struct RandomAchievementConfig;
struct SARandomAchievementClaimConformed;
struct SAUpdateRandomAchievementState;
struct SANewRandomAchievementGot;

class CRandomAchieveManager :
	public CRandomAchieveUpdate
{
public:
	CRandomAchieveManager(CExtendedDataManager& dataMgr);
	virtual ~CRandomAchieveManager();

	///���տͻ��˷�������Ϣ
	void RecvMsg(const SMessage *pMsg);

	///@brief ������ͨ��������Ӣ����������˵���ؾ�Ѱ���ĳɾ���ص���Ϣ
	///@param curConquredTollgateID ��ǰ�����ĸ����ؿ�ID
	virtual void UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes = 1);
	///@brief �����佫�ռ��ĳɾ�
	virtual void UpdateHeroCollectionAchieve();
	///@brief �����佫���׵ĳɾ�
	///@param curHeroRank ��ǰ�佫���׵��ĵȼ�
	virtual void  UpdateHeroRankRiseAchieve(int curHeroRank);
	///@brief ���¾��������ĳɾ�
	///@param curMasterLevel ��ǰ�ľ����ȼ�
	///@param levelUpgradeAmount ���������ĵȼ�����һ����˵Ϊ1��Ҳ���������һ����������
	virtual void UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount);
	///@brief �������ĳɾʹ����Ϣ
	virtual void UpdateBlessAchieve(int blessTimes = 1);
	///@brief ���¼��������ĳɾʹ����Ϣ
	virtual void UpdateSkillUpgradeAchieve(int upgradeTimes = 1);
	///@brief ���¶��������ɾ�
	virtual void UpdateForgingAchieve();

private:
	///@brief ��ȡ�ɾ͵���ز���
	void ClaimAchievement(const SRandomAchievementMsg* pMsg);
	///@brief ��ͨ�õ�ĳһ���ɾʹ���߼��Ĵ���
	bool AchievementCompleteProgress(const RandomAchievementConfig* achievementConfig, int times = 1);
	///@brief ����ˢ�¿ɴ�ɵĳɾͣ������µĳɾͷ���������
	void RandomAchieveTrophy(bool sendMsgFlag = true);

	CRandomAchieveDataManager& m_achievementDataMgr;
	CLuckyTimeSystem* m_ptrLuckyTimeSystem;
	shared_ptr<SARandomAchievementClaimConformed> m_ptrAchievementClaimConformedMsg;
	shared_ptr<SAUpdateRandomAchievementState> m_ptrUpdateAchievementStateMsg;
	shared_ptr<SANewRandomAchievementGot> m_ptrNewRandomAchievementGotMsg;
};

