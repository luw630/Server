// ----- CMissionUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/06/19
//   --  Desc:   萌斗三国的任务更新逻辑管理抽象类
// --------------------------------------------------------------------
//   --  用于抽象出成就更新的部分接口，让其他模块只依赖于该层抽象
//---------------------------------------------------------------------   
#pragma once

enum InstanceType;

class CMissionUpdate
{
public:
	CMissionUpdate(){}
	virtual ~CMissionUpdate(){}

	///@brief 更新副本玩法的任务相关数据
	///@param type 要更新的副本类型
	///@param completeTimes 挑战完成的次数
	virtual void UpdateDungeionMission(InstanceType type, int completeTimes) = 0;
	///@brief 更新祈福玩法的任务相关数据
	///@param blessTimes 祈福成功的次数
	virtual void UpdateBlessingMission(int blessTimes) = 0;
	///@brief 更新锻造玩法的任务相关数据
	///@param forgingTimes 锻造成功的次数
	virtual void UpdateForgingMission(int forgingTimes) = 0;
	///@brief 更新技能升级的任务相关数据
	///@param upgradeTimes 升级技能成功的次数
	virtual void UpdateSkillUpgradeMission(int upgradeTimes) = 0;
	///@brief 更新VIP等级的相关任务
	///@param vipLevel VIP当前的等级
	virtual void UpdateVipMission(int vipLevel) = 0;
};