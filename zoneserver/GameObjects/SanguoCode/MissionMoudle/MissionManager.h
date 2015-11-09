// ----- CMissionManager.h -----
//
//   --  Author: Jonson
//   --  Date:   15/01/26
//   --  Desc:   萌斗三国的任务的相关逻辑的管理
// --------------------------------------------------------------------
//   --  主要判断某一个任务是否可以领取，已经领取后的物品添加。以及利用定时器判断午餐等任务是否可以领取，是否可以显示等。
//	 --  简单的任务都是放到客户端来判断是否已经完成，像VIP相关的任务则是放到服务端来判断
//---------------------------------------------------------------------  
#pragma once
#include "CMissionUpdate.h"

///@brief 任务的逻辑管理类
struct SMessage;
struct SMissionMsg;
class CExtendedDataManager;
class CMissionDataManager;
class CMissionActiveness;

class CMissionManager : public CMissionUpdate
{
public:
	CMissionManager(CExtendedDataManager& missionDataMgr, CExtendedDataManager& missionActivenessMgr);
	~CMissionManager();

	void RecvMsg(const SMessage *pMsg);
	///@brief 更新副本玩法的任务相关数据
	///@param type 要更新的副本类型
	///@param completeTimes 挑战完成的次数
	virtual void UpdateDungeionMission(InstanceType type, int completeTimes);
	///@brief 更新祈福玩法的任务相关数据
	///@param blessTimes 祈福成功的次数
	virtual void UpdateBlessingMission(int blessTimes);
	///@brief 更新锻造玩法的任务相关数据
	///@param forgingTimes 锻造成功的次数
	virtual void UpdateForgingMission(int forgingTimes);
	///@brief 更新技能升级的任务相关数据
	///@param upgradeTimes 升级技能成功的次数
	virtual void UpdateSkillUpgradeMission(int upgradeTimes);
	///@brief 更新VIP等级的相关任务
	///@param vipLevel VIP当前的等级
	virtual void UpdateVipMission(int vipLevel);

private:
	///@brief 客户端请求结束某一个任务
	void AskToAccomplishMission(const SMissionMsg* msg);
	///@breif 客户端请求任务活跃度的奖励
	void AskToClaimMissionActivenessRewards(const SMissionMsg* msg);

	CMissionDataManager * m_ptrMissionDataMgr;
	CMissionActiveness * m_ptrMissionActivenessMgr;
};

