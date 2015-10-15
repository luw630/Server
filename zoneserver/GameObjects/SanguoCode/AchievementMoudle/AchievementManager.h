// ----- CAchievementManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   �ȶ������������ɾ͵��������߼��Ĺ���
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
#include"../PUB/Singleton.h"
#include "AchieveUpdate.h"

///@brief �ɾ͵��߼�������
class CExtendedDataManager;
class CAchievementDataManager;
struct SAchivementUnitData;
struct SAAchievementClaimConformed;
struct SAUpdateAchievementState;
struct AchievementConfig;
struct SMessage;
struct SAchievementMsg;

class CAchievementManager : public CAchieveUpdate
{
public:
	CAchievementManager(CExtendedDataManager& dataMgr);
	virtual ~CAchievementManager();

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

private:
	///@brief ��ȡ�ɾ͵���ز���
	void ClaimAchievement(const SAchievementMsg* pMsg);
	///@brief ��ͨ�õ�ĳһ���ɾʹ���߼��Ĵ���
	const SAchivementUnitData* AchievementCompleteProgress(const AchievementConfig* achievementConfig, OUT bool& achievementAccomplished, int completedTimes = 1);
	///@brief ������һ�������еĳɾ͵���ɴ����жϵ�"����",�����޷�������������ĳɾ͵ļ����߼�
	void ActiveNextAchievement(const SAchivementUnitData* curAchievementData);

	CAchievementDataManager& m_achievementDataMgr;
	shared_ptr<SAAchievementClaimConformed> m_ptrAchievementClaimConformedMsg;
	shared_ptr<SAUpdateAchievementState> m_ptrUpdateAchievementStateMsg;
};

