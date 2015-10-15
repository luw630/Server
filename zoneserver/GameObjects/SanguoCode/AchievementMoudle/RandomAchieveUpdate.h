// ----- CRandomAchieveUpdate.h -----
//
//   --  Author: Jonson
//   --  Date:   15/04/09
//   --  Desc:   萌斗三国的随机成就更新逻辑管理抽象类
// --------------------------------------------------------------------
//   --  用于抽象出成就更新的部分接口，让其他模块只依赖于该层抽象
//---------------------------------------------------------------------   
#pragma once
#include "AchieveUpdate.h"

class CRandomAchieveUpdate :
	public CAchieveUpdate
{
public:
	CRandomAchieveUpdate(){}
	virtual ~CRandomAchieveUpdate(){}

	///@brief 更新祈福的成就达成信息
	virtual void UpdateBlessAchieve(int blessTimes = 1) = 0;
	///@brief 更新技能升级的成就达成信息
	virtual void UpdateSkillUpgradeAchieve(int upgradeTimes = 1) = 0;
	///@breif 更新锻造随机成就的达成情况
	virtual void UpdateForgingAchieve() = 0;
};

