// ----- CRandomAchieveDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   萌斗三国的随机生成的可达成的成就的相关相关数据的管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include <unordered_map>
#include <unordered_set>
#include "..\ExtendedDataManager.h"

struct SAchivementUnitData;
enum RandomAchievementType;

class CRandomAchieveDataManager :
	public CExtendedDataManager
{
public:
	CRandomAchieveDataManager(CBaseDataManager& baseDataMgr);
	virtual ~CRandomAchieveDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 设置成就的完成次数
	///@param missionID 成就对应的ID
	///@return 成功放回修改后的成就数据，失败返回null
	const SAchivementUnitData* SetAchievementCompleteTimes(DWORD achievementID, int times = 1);
	///@brief 设置某一任务已经领取了，跟“达成”的概念有些区别
	void SetAchievementAccompulished(DWORD achievementID);
	///@brief 设置新的可达成的一系列的成就
	///@param achievementIDs 新的成就的ID列表
	void SetNewAchievement(DWORD achievementID);
	///@brief 设置随机生成的可达成的成就的离结束的剩余时间或者离开始的剩余时间
	///@param bLuckyTime 是否处于成就激活的状态，为false就为“白天”，就设置还有多长时间到达成就的激活时间（黑夜）
	///@param remainingTime 剩余时间
	void SetRemainingTimes(bool bLuckyTime, DWORD remainingTime);

	///@brief 获取对应成就的数据
	const SAchivementUnitData* GetAchievementUnitData(DWORD achievementID);
	///@brief 根据随机生成的成就的类型来获取这类的当前能利用的的成就
	///@param type 成就对应的类型
	///@param achievementID 存在的成就的ID
	bool IsAchievementExist(RandomAchievementType type, OUT int& achievementID);
	///@brief 用来刷新远征关卡的票数
	void RefreshData(int sendMsgFlag = 0);
	///@brief 在“白天”的时候重置随机成就的数据
	void LogoutProcess();

private:
	BYTE* m_ptrLuckyTimeFalling;
	DWORD* m_ptrCurRandomAchievementRemainingTime;
	DWORD* m_ptrAccomplishedTimes;
	SAchivementUnitData* m_ptrAchievementUnitData;
};

