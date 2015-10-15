// ----- CAchievementManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的终生成就的相关相关逻辑的管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <memory>
#include"../PUB/Singleton.h"
#include "AchieveUpdate.h"

///@brief 成就的逻辑管理类
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

	///@brief 更新普通副本、精英副本、将神传说、秘境寻宝的成就相关的信息
	///@param curConquredTollgateID 当前征服的副本关卡ID
	virtual void UpdateDuplicateAchieve(int curConquredTollgateID, int completedTimes = 1);
	///@brief 更新武将收集的成就
	virtual void UpdateHeroCollectionAchieve();
	///@brief 更新武将进阶的成就
	///@param curHeroRank 当前武将进阶到的等级
	virtual void  UpdateHeroRankRiseAchieve(int curHeroRank);
	///@brief 更新君主升级的成就
	///@param curMasterLevel 当前的君主等级
	///@param levelUpgradeAmount 君主上升的等级数，一般来说为1，也有特殊情况一次升几级的
	virtual void UpdateMasterLevelAchieve(int curMasterLevel, int levelUpgradeAmount);

private:
	///@brief 领取成就的相关操作
	void ClaimAchievement(const SAchievementMsg* pMsg);
	///@brief 较通用的某一个成就达成逻辑的处理
	const SAchivementUnitData* AchievementCompleteProgress(const AchievementConfig* achievementConfig, OUT bool& achievementAccomplished, int completedTimes = 1);
	///@brief 激活下一个该组中的成就的完成次数判断的"开关",主管无法“跳级”激活的成就的激活逻辑
	void ActiveNextAchievement(const SAchivementUnitData* curAchievementData);

	CAchievementDataManager& m_achievementDataMgr;
	shared_ptr<SAAchievementClaimConformed> m_ptrAchievementClaimConformedMsg;
	shared_ptr<SAUpdateAchievementState> m_ptrUpdateAchievementStateMsg;
};

