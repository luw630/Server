// ----- CAchievementDataManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   萌斗三国的成就的相关数据的管理
// --------------------------------------------------------------------
//---------------------------------------------------------------------  
#pragma once
#include "..\ExtendedDataManager.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

///@brief 成就的数据管理类
enum AchievementType;
struct SAchivementUnitData;
struct SAchivementGroupData;
class CAchievementDataManager :
	public CExtendedDataManager
{
public:
	CAchievementDataManager(CBaseDataManager& baseDataManager);
	virtual ~CAchievementDataManager();

	virtual bool InitDataMgr(void * pData);
	virtual bool ReleaseDataMgr();

	///@brief 设置成就的完成次数
	///@param missionID 成就对应的ID
	///@param bAchievementAccomplished 成就是否“达成”
	///@return 成功放回修改后的成就数据，失败返回null
	const SAchivementUnitData* SetAchievementCompleteTimes(DWORD achievementID, OUT bool& bAchievementAccomplished, int times = 1);
	///@brief 设置某一任务已经领取了，跟“达成”的概念有些区别
	void SetAchievementAccompulished(DWORD achievementID);
	///@brief 设置某一组成就是否全部领取完毕，跟“全部达成”的概念还是有区别
	void SetAchievementGroupAccompulished(AchievementType achievementType);
	///@brief 设置某一组成就的最新的“未领取”的成就ID，跟“未达成”的概念有些区别
	void SetAchievementGroupNewsetID(AchievementType achievementType, DWORD ID);
	///@brief 设置当前的某一组别的成就的 最新的“待达成”的成就ID。此处的"待达成"跟“待领取”的概念有区别,达成后才能领取
	void SetAchievementGroupCurUnaccomplishedID(AchievementType achievmentType, DWORD ID);

	///@brief 获取对应组别的成就的数据
	const SAchivementGroupData* GetAchievementGroupData(AchievementType achievementType);
	///@brief 获取对应成就的数据
	const SAchivementUnitData* GetAchievementUnitData(DWORD achievementID);
	///@brief 获取当前某一组别的成就的 最新的“待达成”的成就ID。此处的"待达成"跟“待领取”的概念有区别,达成后才能领取
	///@return 成功返回对应的值，失败返回-1
	int GetAchievemntGroupCurUnaccomplishedID(AchievementType achievmentType);

private:
	unordered_map<DWORD, SAchivementUnitData*> m_AchievementUnitData;
	unordered_map<DWORD, SAchivementGroupData*> m_AchievementGroupData;
	unordered_map<DWORD, DWORD> m_AchievementGroupCurUnaccomplishedID; ///<某一组成就当前的待完成的成就ID
	unordered_set<DWORD> m_curUnaccomplishedAchievementGroup;///<当前尚未全部完成的成就组
};

