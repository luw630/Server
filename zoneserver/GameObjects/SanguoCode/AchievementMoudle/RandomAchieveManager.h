// ----- CRandomAchieveManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   萌斗三国的随机生成的可达成的成就的相关相关逻辑的管理
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

	///接收客户端发来的消息
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
	///@brief 更新祈福的成就达成信息
	virtual void UpdateBlessAchieve(int blessTimes = 1);
	///@brief 更新技能升级的成就达成信息
	virtual void UpdateSkillUpgradeAchieve(int upgradeTimes = 1);
	///@brief 更新锻造的随机成就
	virtual void UpdateForgingAchieve();

private:
	///@brief 领取成就的相关操作
	void ClaimAchievement(const SRandomAchievementMsg* pMsg);
	///@brief 较通用的某一个成就达成逻辑的处理
	bool AchievementCompleteProgress(const RandomAchievementConfig* achievementConfig, int times = 1);
	///@brief 重新刷新可达成的成就，并将新的成就发往服务器
	void RandomAchieveTrophy(bool sendMsgFlag = true);

	CRandomAchieveDataManager& m_achievementDataMgr;
	CLuckyTimeSystem* m_ptrLuckyTimeSystem;
	shared_ptr<SARandomAchievementClaimConformed> m_ptrAchievementClaimConformedMsg;
	shared_ptr<SAUpdateRandomAchievementState> m_ptrUpdateAchievementStateMsg;
	shared_ptr<SANewRandomAchievementGot> m_ptrNewRandomAchievementGotMsg;
};

